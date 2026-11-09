/*
 * Emergency-path self-test (CB-50..CB-53): closes the latent-failure rows of
 * the FMEA (relay stuck energised, emergency amplifier dead).
 *
 * Hardware assumption (DOSSIER §6, decision D7): with the relay released, the
 * codec output reaches the emergency line through the relay's rest contact,
 * and the emergency amplifier's output is wired back to a spare ADC input
 * (aux). The test plays a 1 kHz tone at 0.1 of full scale for ICOM_BT_TICKS
 * ms and measures the loopback mean square after ICOM_BT_SETTLE ms. It passes
 * when the loopback is within +/-12 dB of the nominal unity-gain level.
 * SPDX-License-Identifier: MIT
 */
#include <math.h>

#include "icom/icom.h"

#define BT_AMP 0.1f
#define BT_FREQ_HZ 1000.0f
#define TWO_PI_F 6.28318530717958647692f
#define BT_MS_NOM ((BT_AMP * BT_AMP) / 2.0f)
#define BT_RANGE 15.85f /* 12 dB as a power ratio */

void icom_bt_init(icom_bt_t *t)
{
	t->running = false;
	t->t = 0U;
	t->phase = 0.0f;
	t->acc = 0.0f;
	t->n = 0U;
	t->result = ICOM_BT_NONE;
}

void icom_bt_start(icom_bt_t *t)
{
	icom_bt_init(t);
	t->running = true;
}

void icom_bt_abort(icom_bt_t *t)
{
	t->running = false;
	t->result = ICOM_BT_NONE;
}

void icom_bt_frame(icom_bt_t *t, const int16_t aux[ICOM_N], int16_t out[ICOM_N])
{
	uint32_t i;

	for (i = 0U; i < ICOM_N; i++) {
		t->phase += TWO_PI_F * BT_FREQ_HZ / (float)ICOM_FS_HZ;
		if (t->phase >= TWO_PI_F) {
			t->phase -= TWO_PI_F;
		}
		const float tone = BT_AMP * sinf(t->phase) * 32767.0f;
		out[i] = (int16_t)tone;
		if (t->t >= ICOM_BT_SETTLE) {
			const float v = (float)aux[i] / 32768.0f;
			t->acc += v * v;
			t->n++;
		}
	}
	t->t++;
	if (t->t >= ICOM_BT_TICKS) {
		const float ms = t->acc / (float)t->n;
		t->running = false;
		if ((ms > (BT_MS_NOM / BT_RANGE)) && (ms < (BT_MS_NOM * BT_RANGE))) {
			t->result = ICOM_BT_PASS;
		} else {
			t->result = ICOM_BT_FAIL;
		}
	}
}
