/*
 * Link supervision and loss concealment tests (CB-14..CB-16).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static icom_down_t mk(uint8_t seq, uint8_t nch, int16_t v)
{
	icom_down_t d;
	memset(&d, 0, sizeof(d));
	d.seq = seq;
	d.flags = 0x09;
	d.nch = nch;
	for (unsigned c = 0; c < ICOM_NCH; c++)
		for (unsigned i = 0; i < ICOM_N; i++) d.pcm[c][i] = v;
	return d;
}

void test_link(void)
{
	icom_link_t l;
	icom_chans_t pc;
	icom_link_init(&l);

	icom_down_t d = mk(10, 2, 1600);
	CHECK_EQ(icom_link_tick(&l, &d, &pc), 0x09);
	CHECK_EQ(pc.s[0][0], 1600);
	CHECK_EQ(pc.s[1][5], 1600);
	CHECK_EQ(pc.s[2][0], 0); /* channels not in the frame are silent */
	CHECK_EQ(l.good_run, 1);
	CHECK_EQ(l.lost_total, 0);

	d = mk(13, 4, 1600); /* 11 and 12 lost */
	icom_link_tick(&l, &d, &pc);
	CHECK_EQ(l.lost_total, 2);
	CHECK_EQ(l.good_run, 2);

	/* concealment: 3 frames at -6 dB steps, then mute (CB-15) */
	CHECK_EQ(icom_link_tick(&l, NULL, &pc), 0x09);
	CHECK_EQ(pc.s[3][0], 800);
	CHECK_EQ(icom_link_tick(&l, NULL, &pc), 0x09);
	CHECK_EQ(pc.s[3][0], 400);
	CHECK_EQ(icom_link_tick(&l, NULL, &pc), 0x09);
	CHECK_EQ(pc.s[3][0], 200);
	CHECK_EQ(icom_link_tick(&l, NULL, &pc), 0x00);
	CHECK_EQ(pc.s[3][0], 0);
	CHECK_EQ(l.missed_run, 4);
	CHECK_EQ(l.good_run, 0);
	CHECK_EQ(l.concealed, 3);
	CHECK_EQ(l.lost_total, 6);

	/* sequence wraps 255 -> 0 without a false loss */
	d = mk(255, 0, 0);
	icom_link_tick(&l, &d, &pc);
	CHECK_EQ(l.missed_run, 0);
	unsigned before = l.lost_total;
	d = mk(0, 0, 0);
	icom_link_tick(&l, &d, &pc);
	CHECK_EQ(l.lost_total, before);
}
