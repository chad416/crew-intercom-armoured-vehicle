/*
 * Crew box top level: bus in, modes, ANR, mixer, uplink (CB-01..CB-05 and
 * the integration of the others).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static icom_box_t box;
static plant_t pl;
static int16_t mic[ICOM_N], out[ICOM_N], errs[ICOM_N];
static uint8_t up[ICOM_MAX_FRAME];
static uint8_t seq;
static icom_up_t last_up;

static void send(uint16_t speed_ckmh, uint8_t cmd, int16_t arg)
{
	icom_down_t d;
	uint8_t buf[ICOM_MAX_FRAME];
	memset(&d, 0, sizeof(d));
	d.seq = seq++;
	d.flags = 0x08;
	d.nch = 4;
	d.speed_ckmh = speed_ckmh;
	d.cmd = cmd;
	d.arg = arg;
	uint32_t n = icom_down_encode(&d, buf, sizeof(buf));
	icom_box_rx(&box, buf, n);
}

/* one tick: the plant turns the previous earphone output into the error mic */
static uint32_t tick(uint8_t bits, double speed_kmh)
{
	float anti[ICOM_N], err[ICOM_N];
	for (unsigned i = 0; i < ICOM_N; i++) anti[i] = (float)out[i] / 32768.0f;
	plant_frame(&pl, speed_kmh, anti, err, NULL);
	for (unsigned i = 0; i < ICOM_N; i++) errs[i] = (int16_t)(err[i] * 32767.0f);
	uint32_t n = icom_box_tick(&box, mic, errs, bits, out, up, sizeof(up));
	if (n) {
		icom_parser_t p;
		icom_down_t dd;
		icom_parser_init(&p);
		for (uint32_t i = 0; i < n; i++) icom_parser_feed(&p, up[i], &dd, &last_up);
	}
	return n;
}

static void run(unsigned ticks, uint16_t speed_ckmh, int frames, uint8_t bits)
{
	for (unsigned t = 0; t < ticks; t++) {
		if (frames) send(speed_ckmh, ICOM_CMD_NONE, 0);
		tick(bits, speed_ckmh / 100.0);
	}
}

void test_box(void)
{
	static const double amp[3] = {0.10, 0.07, 0.05};
	icom_box_init(&box, 5);
	/* the box's default S_hat is a 3 ms delay: plant 2 ms + 1 frame */
	plant_init(&pl, 0.15, 3, amp, 32, 1.0);
	memset(mic, 0, sizeof(mic));
	memset(out, 0, sizeof(out));

	/* INIT: no uplink until the link is up */
	CHECK_EQ(tick(0, 0.0), 0);
	CHECK_EQ(box.modes.state, ICOM_ST_INIT);

	/* NORMAL with microphone audio in the uplink */
	send(5000, ICOM_CMD_NONE, 0);
	CHECK_EQ(tick(0, 50.0), ICOM_HDR_LEN + ICOM_UP_FIX + ICOM_N * 2u + ICOM_CRC_LEN);
	CHECK_EQ(box.modes.state, ICOM_ST_NORMAL);
	CHECK_EQ(last_up.box_id, 5);
	CHECK_EQ(last_up.state, ICOM_ST_NORMAL);
	CHECK_EQ(last_up.nch, 1);

	/* ANR converges in the box loop; telemetry rotates through 4 items */
	run(3000, 5000, 1, 0);
	int seen_att = 0;
	for (int t = 0; t < 4; t++) {
		run(1, 5000, 1, 0);
		if (last_up.tele_id == 1) {
			seen_att = 1;
			printf("  box ANR telemetry: %.1f dB\n", last_up.tele / 10.0);
			CHECK(last_up.tele > 250);
		}
	}
	CHECK(seen_att);

	/* CB-22: 20 ms without frames -> BYPASS; telemetry-only uplink */
	run(ICOM_T_LOSS_MS - 1, 5000, 0, 0);
	CHECK_EQ(box.modes.state, ICOM_ST_NORMAL);
	CHECK_EQ(tick(0, 50.0), ICOM_HDR_LEN + ICOM_UP_FIX + ICOM_CRC_LEN);
	CHECK_EQ(box.modes.state, ICOM_ST_BYPASS);
	CHECK_EQ(last_up.nch, 0);
	CHECK(!box.anr.enabled);

	/* CB-23: back to NORMAL after 2 s of good frames */
	run(ICOM_T_RECOVER_MS, 5000, 1, 0);
	CHECK_EQ(box.modes.state, ICOM_ST_NORMAL);

	/* uplink frames from other boxes on the bus are ignored */
	{
		icom_up_t u;
		uint8_t buf[ICOM_MAX_FRAME];
		memset(&u, 0, sizeof(u));
		uint32_t n = icom_up_encode(&u, buf, sizeof(buf));
		icom_box_rx(&box, buf, n);
		CHECK(!box.rx_valid);
	}

	/* S_hat rotation is refused outside MAINT */
	float re0 = box.anr.sh_re[3];
	send(5000, ICOM_CMD_SHAT_ROT, 150);
	tick(0, 50.0);
	CHECK_EQ(box.anr.sh_re[3], re0);

	/* MAINT at standstill; inject a 150 deg S_hat error (fault injection) */
	run(200, 0, 1, 0);
	send(0, ICOM_CMD_MAINT_ENTER, 0);
	tick(0, 0.0);
	CHECK_EQ(box.modes.state, ICOM_ST_MAINT);
	send(0, ICOM_CMD_SHAT_ROT, 150);
	tick(0, 0.0);
	CHECK(box.anr.sh_re[3] != re0);
	send(0, ICOM_CMD_MAINT_EXIT, 0);
	tick(0, 0.0);
	CHECK_EQ(box.modes.state, ICOM_ST_NORMAL);

	/* CB-46: the wrong S_hat makes the ANR diverge; the guard trips -> DEGRADED */
	int t_div = 0;
	for (; t_div < 4000 && !box.anr.fault; t_div++) run(1, 5000, 1, 0);
	printf("  box ANR guard tripped after %d ms\n", t_div);
	run(1, 5000, 1, 0);
	CHECK(box.anr.fault);
	CHECK_EQ(box.modes.state, ICOM_ST_DEGRADED);
	CHECK(last_up.bits & ICOM_BIT_ANR);
	CHECK_EQ(out[0], 0); /* no anti-noise: earphone silent (no downlink audio) */

	/* repair in MAINT (entered from DEGRADED): rotate back, reset, exit -> NORMAL */
	run(10, 0, 1, 0);
	send(0, ICOM_CMD_MAINT_ENTER, 0);
	tick(0, 0.0);
	CHECK_EQ(box.modes.state, ICOM_ST_MAINT);
	send(0, ICOM_CMD_SHAT_ROT, -150);
	tick(0, 0.0);
	CHECK_NEAR(box.anr.sh_re[3], re0, 1e-4);
	send(0, ICOM_CMD_MAINT_EXIT, 0);
	tick(0, 0.0);
	CHECK_EQ(box.modes.state, ICOM_ST_DEGRADED); /* the ANR fault is still latched */
	send(0, ICOM_CMD_ANR_RESET, 0);
	tick(0, 0.0);
	tick(0, 0.0); /* no frame this tick */
	run(2, 0, 1, 0);
	CHECK_EQ(box.modes.state, ICOM_ST_NORMAL);
	run(1000, 5000, 1, 0);
	CHECK(box.anr.enabled);
	CHECK(!box.anr.fault);

	/* a corrupted frame is counted (telemetry item 3) */
	{
		icom_down_t d;
		uint8_t buf[ICOM_MAX_FRAME];
		memset(&d, 0, sizeof(d));
		uint32_t n = icom_down_encode(&d, buf, sizeof(buf));
		buf[7] ^= 0x10;
		icom_box_rx(&box, buf, n);
		CHECK_EQ(box.parser.crc_errors, 1);
	}
	for (int t = 0; t < 4; t++) {
		run(1, 0, 1, 0);
		if (last_up.tele_id == 3) CHECK_EQ(last_up.tele, 1);
		if (last_up.tele_id == 0) CHECK_EQ(last_up.tele >> 8, ICOM_WHY_NONE);
	}

	/* CB-24: critical BIT -> latched BYPASS */
	run(5, 0, 1, ICOM_BIT_CODEC);
	CHECK_EQ(box.modes.state, ICOM_ST_BYPASS);
	CHECK(box.modes.latched);
	run(3000, 0, 1, 0);
	CHECK_EQ(box.modes.state, ICOM_ST_BYPASS);
}
