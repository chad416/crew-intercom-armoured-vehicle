/*
 * Priority mixer, sidetone and limiter (CB-30..CB-35).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static icom_chans_t pc;
static int16_t mic[ICOM_N];
static int16_t out[ICOM_N];

static void set_ch(unsigned c, int16_t v)
{
	for (unsigned i = 0; i < ICOM_N; i++) pc.s[c][i] = (int16_t)((i & 1u) ? -v : v);
}

void test_mix(void)
{
	icom_mix_t x;
	uint8_t act;
	float anti[ICOM_N];

	icom_mix_init(&x);
	memset(&pc, 0, sizeof(pc));
	memset(mic, 0, sizeof(mic));

	/* radio 1 alone: full volume */
	set_ch(ICOM_CH_RADIO1, 3000);
	act = icom_mix_frame(&x, &pc, 0x02, mic, NULL, out);
	CHECK_EQ(act, 0x02);
	CHECK_EQ(out[0], 3000);

	/* intercom becomes active: radio ducked by 0.25 = -12 dB (CB-31) */
	set_ch(ICOM_CH_ICOM, 2000);
	act = icom_mix_frame(&x, &pc, 0x0A, mic, NULL, out);
	CHECK_EQ(act, 0x0A);
	CHECK_EQ(out[0], 2000 + 750);

	/* alarm overrides both (CB-30) */
	set_ch(ICOM_CH_ALARM, 4000);
	icom_mix_frame(&x, &pc, 0x0B, mic, NULL, out);
	CHECK_EQ(out[0], 4000 + 500 + 750);

	/* alarm silent but flag still set: held for hold_ms (CB-32) */
	set_ch(ICOM_CH_ALARM, 0);
	act = icom_mix_frame(&x, &pc, 0x0B, mic, NULL, out);
	CHECK(act & 0x01);
	for (unsigned t = 0; t < x.hold_ms; t++) act = icom_mix_frame(&x, &pc, 0x0B, mic, NULL, out);
	CHECK_EQ(act & 0x01, 0);
	CHECK_EQ(out[0], 2000 + 750);
	/* flag cleared: channel off at once */
	act = icom_mix_frame(&x, &pc, 0x02, mic, NULL, out);
	CHECK_EQ(act, 0x02);
	CHECK_EQ(out[0], 3000);

	/* sidetone: local mic at 0.25 (CB-33) */
	memset(&pc, 0, sizeof(pc));
	for (unsigned i = 0; i < ICOM_N; i++) mic[i] = 4000;
	act = icom_mix_frame(&x, &pc, 0x00, mic, NULL, out);
	CHECK_EQ(act, 0);
	CHECK_EQ(out[3], 1000);
	memset(mic, 0, sizeof(mic));

	/* limiter: peak held to the limit, then releases (CB-34) */
	icom_mix_init(&x);
	set_ch(ICOM_CH_ICOM, 30000);
	icom_mix_frame(&x, &pc, 0x08, mic, NULL, out);
	CHECK_NEAR(out[0], 16384, 2);
	CHECK_NEAR(out[1], -16384, 2);
	set_ch(ICOM_CH_ICOM, 1000);
	for (int t = 0; t < 1000; t++) icom_mix_frame(&x, &pc, 0x08, mic, NULL, out);
	CHECK_NEAR(out[0], 1000, 2);
	/* a frame that starts with a negative sample sets the peak from below */
	set_ch(ICOM_CH_ICOM, -30000);
	icom_mix_frame(&x, &pc, 0x08, mic, NULL, out);
	CHECK_NEAR(out[0], -16384, 2);

	/* anti-noise is added after the limiter; the output saturates (CB-35) */
	for (unsigned i = 0; i < ICOM_N; i++) anti[i] = 0.9f;
	set_ch(ICOM_CH_ICOM, 30000);
	icom_mix_frame(&x, &pc, 0x08, mic, anti, out);
	CHECK_EQ(out[0], 32767);
	for (unsigned i = 0; i < ICOM_N; i++) anti[i] = -0.9f;
	icom_mix_frame(&x, &pc, 0x08, mic, anti, out);
	CHECK_EQ(out[1], -32768);
	for (unsigned i = 0; i < ICOM_N; i++) anti[i] = 0.1f;
	set_ch(ICOM_CH_ICOM, 0);
	icom_mix_frame(&x, &pc, 0x08, mic, anti, out);
	CHECK_NEAR(out[0], 3277, 1);
}
