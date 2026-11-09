/*
 * Crew-box firmware: Zephyr glue around lib/icom.
 *
 *  - bus UART (LPUART3, RS-485 on a real unit): RX ISR -> ring -> icom_box_rx
 *  - audio thread, one 1 ms frame per tick (k_timer stands in for the codec's
 *    DMA half-transfer interrupt): icom_box_tick, uplink frame out, bypass
 *    relay drive (toggling charge-pump input on GPIO1_IO09)
 *  - codec port: in the test image, an acoustic plant model (track tones from
 *    the bus speed + the delayed anti-noise) replaces the SAI/codec, which the
 *    Renode RT1064 model does not provide (see README, limits)
 *
 * SPDX-License-Identifier: MIT
 */
#include <math.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/ring_buffer.h>

#include "icom/icom.h"

#define USER_NODE DT_PATH(zephyr_user)
static const struct gpio_dt_spec relay = GPIO_DT_SPEC_GET(USER_NODE, relay_gpios);
static const struct device *const bus_uart = DEVICE_DT_GET(DT_CHOSEN(icom_bus_uart));

static icom_box_t box;

/* ---- Diagnostics, read by the test rig through the ELF symbols ------------ */

volatile uint32_t icom_diag_ticks;
volatile uint32_t icom_diag_max_cyc; /* worst icom_box_tick time, CPU cycles */
volatile uint32_t icom_diag_state;
volatile uint32_t icom_diag_rx_overflow;

#ifdef CONFIG_ICOM_TEST_HOOKS
volatile uint32_t icom_test_bits;  /* BIT bits injected by the rig */
volatile uint32_t icom_test_stall; /* 1: the audio thread hangs (processor fault) */
volatile int32_t icom_test_out_peak; /* largest earphone sample since boot */
/* emergency-line loopback gain in percent (100 = healthy; 0 = relay stuck or amplifier dead) */
volatile uint32_t icom_test_loop_pct = 100U;

/* Acoustic plant (DOSSIER §9): 3 track harmonics at the error microphone, plus
 * the anti-noise delayed by 32 samples (2 ms of converters and acoustics; the
 * frame pipeline adds 1 ms, total 3 ms = the box's default S_hat). */
#define PLANT_D 32U
#define PLANT_LINE 64U
static float plant_line[PLANT_LINE];
static uint32_t plant_idx;
static float plant_phase;
static int16_t last_out[ICOM_N];

static void plant_frame(float speed_kmh, int16_t err[ICOM_N])
{
	static const float amp[3] = {0.10f, 0.07f, 0.05f};
	static const float ph0[3] = {0.7f, 1.4f, 2.1f};
	const float f0 = (speed_kmh / 3.6f) / 0.15f;

	for (uint32_t i = 0; i < ICOM_N; i++) {
		float d = 0.0f;

		plant_phase += 6.2831853f * f0 / (float)ICOM_FS_HZ;
		if (plant_phase > 6.2831853f) {
			plant_phase -= 6.2831853f;
		}
		for (uint32_t h = 0; h < 3U; h++) {
			d += amp[h] * cosf((float)(h + 1U) * plant_phase + ph0[h]);
		}
		plant_line[plant_idx % PLANT_LINE] = (float)last_out[i] / 32768.0f;
		const float anti = plant_line[(plant_idx + PLANT_LINE - PLANT_D) % PLANT_LINE];
		plant_idx++;
		float e = (d + anti) * 32767.0f;
		e = (e > 32767.0f) ? 32767.0f : ((e < -32768.0f) ? -32768.0f : e);
		err[i] = (int16_t)e;
	}
}
#endif

/* ---- Bus receive ------------------------------------------------------------ */

RING_BUF_DECLARE(rx_ring, 1024);

static void uart_isr(const struct device *dev, void *user)
{
	ARG_UNUSED(user);
	uart_irq_update(dev);
	while (uart_irq_rx_ready(dev)) {
		uint8_t buf[32];
		const int n = uart_fifo_read(dev, buf, sizeof(buf));

		if (n <= 0) {
			break;
		}
		if (ring_buf_put(&rx_ring, buf, (uint32_t)n) < (uint32_t)n) {
			icom_diag_rx_overflow++;
		}
	}
}

/* ---- Audio thread ------------------------------------------------------------ */

static K_SEM_DEFINE(tick_sem, 0, 1);

static void tick(struct k_timer *t)
{
	ARG_UNUSED(t);
	k_sem_give(&tick_sem);
}

static K_TIMER_DEFINE(tick_timer, tick, NULL);

static void audio_thread(void *a, void *b, void *c)
{
	static int16_t mic[ICOM_N];
	static int16_t err[ICOM_N];
	static int16_t out[ICOM_N];
	static int16_t aux[ICOM_N]; /* emergency-line loopback ADC input */
	static uint8_t up[ICOM_MAX_FRAME];
	uint8_t rx[64];

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);
	k_timer_start(&tick_timer, K_MSEC(1), K_MSEC(1));
	for (;;) {
		uint32_t n;
		uint8_t bits = 0U;

		k_sem_take(&tick_sem, K_FOREVER);
#ifdef CONFIG_ICOM_TEST_HOOKS
		while (icom_test_stall != 0U) {
			/* processor fault: nothing else of this thread runs */
		}
		bits = (uint8_t)icom_test_bits;
		plant_frame((float)box.speed_cckmh / 100.0f, err);
		for (uint32_t i = 0; i < ICOM_N; i++) {
			/* loopback of the last earphone frame, only while the relay is released */
			aux[i] = box.bt.running ? (int16_t)(((int32_t)last_out[i] * (int32_t)icom_test_loop_pct) / 100) : 0;
		}
#endif
		while ((n = ring_buf_get(&rx_ring, rx, sizeof(rx))) > 0U) {
			icom_box_rx(&box, rx, n);
		}
		const uint32_t t0 = k_cycle_get_32();
		const uint32_t len = icom_box_tick(&box, mic, err, aux, bits, out, up, sizeof(up));
		const uint32_t dt = k_cycle_get_32() - t0;

		(void)gpio_pin_set_dt(&relay, icom_box_relay_drive(&box) ? 1 : 0);
		for (uint32_t i = 0; i < len; i++) {
			uart_poll_out(bus_uart, up[i]);
		}
		if (dt > icom_diag_max_cyc) {
			icom_diag_max_cyc = dt;
		}
		icom_diag_state = (uint32_t)box.modes.state;
		icom_diag_ticks++;
#ifdef CONFIG_ICOM_TEST_HOOKS
		memcpy(last_out, out, sizeof(out));
		for (uint32_t i = 0; i < ICOM_N; i++) {
			const int32_t v = (out[i] < 0) ? -(int32_t)out[i] : (int32_t)out[i];
			if (v > icom_test_out_peak) {
				icom_test_out_peak = v;
			}
		}
#endif
	}
}

K_THREAD_STACK_DEFINE(audio_stack, 4096);
static struct k_thread audio_data;

int main(void)
{
	int err = gpio_pin_configure_dt(&relay, GPIO_OUTPUT_INACTIVE);

	if ((err != 0) || !device_is_ready(bus_uart)) {
		printk("icom: I/O init failed (%d)\n", err);
		return 0;
	}
	icom_box_init(&box, 1U);
#ifdef CONFIG_ICOM_TEST_HOOKS
	box.up_audio = false; /* telemetry-only uplink keeps the Renode rig fast */
#endif
	uart_irq_callback_user_data_set(bus_uart, uart_isr, NULL);
	uart_irq_rx_enable(bus_uart);
	k_thread_create(&audio_data, audio_stack, K_THREAD_STACK_SIZEOF(audio_stack), audio_thread, NULL,
			NULL, NULL, 2, 0, K_NO_WAIT);
	k_thread_name_set(&audio_data, "audio");
	printk("icom READY\n");
	return 0;
}
