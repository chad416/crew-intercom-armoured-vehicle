/*
 * Bus framing tests (CB-10..CB-13).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static void fill_down(icom_down_t *d, uint8_t nch)
{
	memset(d, 0, sizeof(*d));
	d->seq = 7;
	d->ccu_mode = ICOM_CCU_REBROADCAST;
	d->flags = 0x0B;
	d->speed_ckmh = 5012;
	d->cmd = ICOM_CMD_SHAT_ROT;
	d->arg = -45;
	d->nch = nch;
	for (unsigned c = 0; c < ICOM_NCH; c++)
		for (unsigned i = 0; i < ICOM_N; i++)
			d->pcm[c][i] = (int16_t)((int)(c * 1000 + i * 37) - 2000);
}

static void test_crc(void)
{
	const uint8_t s[] = "123456789";
	CHECK_EQ(icom_crc16(s, 9), 0x29B1); /* CRC-16/CCITT-FALSE check value */
}

static void test_roundtrip(void)
{
	uint8_t buf[ICOM_MAX_FRAME + 8];
	for (uint8_t nch = 0; nch <= ICOM_NCH; nch++) {
		icom_down_t d, r;
		icom_up_t u;
		icom_parser_t p;
		fill_down(&d, nch);
		uint32_t n = icom_down_encode(&d, buf, sizeof(buf));
		CHECK_EQ(n, ICOM_HDR_LEN + ICOM_DOWN_FIX + nch * ICOM_N * 2u + ICOM_CRC_LEN);
		icom_parser_init(&p);
		int got = 0;
		for (uint32_t i = 0; i < n; i++) {
			icom_rx_kind_t k = icom_parser_feed(&p, buf[i], &r, &u);
			if (k == ICOM_RX_DOWN) got++;
			CHECK((k == ICOM_RX_NONE) == (i + 1 < n));
		}
		CHECK_EQ(got, 1);
		CHECK_EQ(r.seq, 7);
		CHECK_EQ(r.ccu_mode, ICOM_CCU_REBROADCAST);
		CHECK_EQ(r.flags, 0x0B);
		CHECK_EQ(r.speed_ckmh, 5012);
		CHECK_EQ(r.cmd, ICOM_CMD_SHAT_ROT);
		CHECK_EQ(r.arg, -45);
		CHECK_EQ(r.nch, nch);
		for (unsigned c = 0; c < nch; c++)
			CHECK(memcmp(r.pcm[c], d.pcm[c], sizeof(d.pcm[c])) == 0);
		CHECK_EQ(p.frames, 1);
	}
	for (uint8_t nch = 0; nch <= 1; nch++) {
		icom_up_t u = {0}, r;
		icom_down_t dd;
		icom_parser_t p;
		u.seq = 200; u.box_id = 3; u.state = ICOM_ST_DEGRADED; u.bits = ICOM_BIT_ANR;
		u.ptt = 1; u.tele_id = 2; u.tele = 0xBEEF; u.nch = nch;
		for (unsigned i = 0; i < ICOM_N; i++) u.pcm[i] = (int16_t)(i * 1000 - 8000);
		uint32_t n = icom_up_encode(&u, buf, sizeof(buf));
		CHECK_EQ(n, ICOM_HDR_LEN + ICOM_UP_FIX + nch * ICOM_N * 2u + ICOM_CRC_LEN);
		icom_parser_init(&p);
		icom_rx_kind_t k = ICOM_RX_NONE;
		for (uint32_t i = 0; i < n; i++) k = icom_parser_feed(&p, buf[i], &dd, &r);
		CHECK_EQ(k, ICOM_RX_UP);
		CHECK_EQ(r.seq, 200); CHECK_EQ(r.box_id, 3); CHECK_EQ(r.state, ICOM_ST_DEGRADED);
		CHECK_EQ(r.bits, ICOM_BIT_ANR); CHECK_EQ(r.ptt, 1); CHECK_EQ(r.tele_id, 2);
		CHECK_EQ(r.tele, 0xBEEF); CHECK_EQ(r.nch, nch);
		if (nch) CHECK(memcmp(r.pcm, u.pcm, sizeof(u.pcm)) == 0);
	}
}

static void test_encode_limits(void)
{
	uint8_t buf[ICOM_MAX_FRAME];
	icom_down_t d;
	icom_up_t u = {0};
	fill_down(&d, 4);
	CHECK_EQ(icom_down_encode(&d, buf, 10), 0);          /* buffer too small */
	d.nch = ICOM_NCH + 1;
	CHECK_EQ(icom_down_encode(&d, buf, sizeof(buf)), 0); /* too many channels */
	u.nch = 2;
	CHECK_EQ(icom_up_encode(&u, buf, sizeof(buf)), 0);
	u.nch = 1;
	CHECK_EQ(icom_up_encode(&u, buf, 5), 0);
}

/* R: every single-bit error in a frame is rejected and the next frame is still received */
static void test_bitflips(void)
{
	uint8_t buf[ICOM_MAX_FRAME];
	icom_down_t d, r;
	icom_up_t u;
	fill_down(&d, 2);
	uint32_t n = icom_down_encode(&d, buf, sizeof(buf));
	int accepted_bad = 0, good_after = 0;
	for (uint32_t bit = 0; bit < n * 8; bit++) {
		icom_parser_t p;
		icom_parser_init(&p);
		uint8_t bad[ICOM_MAX_FRAME];
		memcpy(bad, buf, n);
		bad[bit / 8] ^= (uint8_t)(1u << (bit % 8));
		for (uint32_t i = 0; i < n; i++)
			if (icom_parser_feed(&p, bad[i], &r, &u) != ICOM_RX_NONE) accepted_bad++;
		/* a corrupted length byte can make the parser wait for more bytes;
		 * worst case it consumes the start of the next frame, so send three */
		for (int rep = 0; rep < 3; rep++)
			for (uint32_t i = 0; i < n; i++)
				if (icom_parser_feed(&p, buf[i], &r, &u) == ICOM_RX_DOWN) good_after++;
		CHECK(good_after > 0);
		good_after = 0;
	}
	CHECK_EQ(accepted_bad, 0);
}

static void test_parser_errors(void)
{
	icom_parser_t p;
	icom_down_t r;
	icom_up_t u;
	uint8_t buf[ICOM_MAX_FRAME];
	icom_down_t d;
	fill_down(&d, 0);
	uint32_t n = icom_down_encode(&d, buf, sizeof(buf));

	/* garbage, A5 A5 5A double sync, A5 x */
	icom_parser_init(&p);
	const uint8_t junk[] = {0x00, 0xA5, 0x13, 0xA5, 0xA5};
	for (unsigned i = 0; i < sizeof(junk); i++) icom_parser_feed(&p, junk[i], &r, &u);
	icom_rx_kind_t k = ICOM_RX_NONE;
	for (uint32_t i = 1; i < n; i++) k = icom_parser_feed(&p, buf[i], &r, &u); /* 5A ... */
	CHECK_EQ(k, ICOM_RX_DOWN);

	/* length above maximum */
	icom_parser_init(&p);
	icom_parser_feed(&p, 0xA5, &r, &u);
	icom_parser_feed(&p, 0x5A, &r, &u);
	icom_parser_feed(&p, (uint8_t)(ICOM_MAX_PAYLOAD + 1), &r, &u);
	CHECK_EQ(p.format_errors, 1);
	CHECK_EQ(p.pos, 0);

	/* valid CRC, but wrong type / inconsistent length / too many channels */
	struct { uint8_t type; uint8_t len; uint8_t nch; } cases[] = {
		{0x33, ICOM_DOWN_FIX, 0},
		{ICOM_T_DOWN, ICOM_DOWN_FIX - 1, 0},
		{ICOM_T_DOWN, ICOM_DOWN_FIX, 5},
		{ICOM_T_DOWN, ICOM_DOWN_FIX + 1, 0},
		{ICOM_T_UP, ICOM_UP_FIX - 1, 0},
		{ICOM_T_UP, ICOM_UP_FIX, 2},
		{ICOM_T_UP, ICOM_UP_FIX + 1, 0},
	};
	for (unsigned c = 0; c < sizeof(cases) / sizeof(cases[0]); c++) {
		uint8_t f[64] = {ICOM_SYNC0, ICOM_SYNC1, cases[c].len, cases[c].type};
		f[4 + 8] = cases[c].nch;
		uint16_t crc = icom_crc16(&f[2], cases[c].len + 2u);
		f[4 + cases[c].len] = (uint8_t)(crc >> 8);
		f[5 + cases[c].len] = (uint8_t)crc;
		icom_parser_init(&p);
		k = ICOM_RX_NONE;
		for (unsigned i = 0; i < 6u + cases[c].len; i++) k = icom_parser_feed(&p, f[i], &r, &u);
		CHECK_EQ(k, ICOM_RX_NONE);
		CHECK_EQ(p.format_errors, 1);
		CHECK_EQ(p.crc_errors, 0);
	}
}

void test_frame(void)
{
	test_crc();
	test_roundtrip();
	test_encode_limits();
	test_bitflips();
	test_parser_errors();
}
