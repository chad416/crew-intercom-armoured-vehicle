/*
 * CCU core: supervision, mix-minus, radio routing, rebroadcast (CCU-01..CCU-08).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static icom_ccu_t c;
static icom_ccu_in_t in;
static icom_ccu_out_t out;

static void up(uint8_t box, uint8_t ptt, int16_t v)
{
	icom_up_t u;
	memset(&u, 0, sizeof(u));
	u.box_id = box;
	u.state = ICOM_ST_NORMAL;
	u.ptt = ptt;
	u.nch = 1;
	for (unsigned i = 0; i < ICOM_N; i++) u.pcm[i] = v;
	icom_ccu_uplink(&c, &u);
}

static void all_up(void)
{
	for (uint8_t b = 0; b < 4; b++) up(b, 0, 0);
}

void test_ccu(void)
{
	memset(&in, 0, sizeof(in));
	icom_ccu_init(&c, 12, 0); /* clamped to ICOM_MAX_BOX */
	CHECK_EQ(c.n_box, ICOM_MAX_BOX);
	icom_ccu_init(&c, 4, 0);

	/* CCU-02 mix-minus: box 1 and 2 talk; each hears the other, not itself */
	all_up();
	up(1, ICOM_PTT_ICOM, 1000);
	up(2, ICOM_PTT_ICOM, 300);
	in.speed_ckmh = 4200;
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[0].pcm[ICOM_CH_ICOM][0], 1300);
	CHECK_EQ(out.down[1].pcm[ICOM_CH_ICOM][0], 300);
	CHECK_EQ(out.down[2].pcm[ICOM_CH_ICOM][0], 1000);
	CHECK(out.down[0].flags & (1u << ICOM_CH_ICOM));
	CHECK_EQ(out.down[0].nch, ICOM_NCH);
	CHECK_EQ(out.down[3].speed_ckmh, 4200);
	CHECK_EQ(out.lost, 0);
	/* a lone talker gets no intercom flag (nobody else talks) */
	all_up();
	up(1, ICOM_PTT_ICOM, 1000);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[1].flags & (1u << ICOM_CH_ICOM), 0);
	CHECK(out.down[0].flags & (1u << ICOM_CH_ICOM));
	/* PTT without audio (telemetry-only uplink, e.g. box in BYPASS) is not mixed */
	all_up();
	{
		icom_up_t u;
		memset(&u, 0, sizeof(u));
		u.box_id = 3;
		u.ptt = ICOM_PTT_ICOM;
		u.nch = 0;
		icom_ccu_uplink(&c, &u);
	}
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[0].flags & (1u << ICOM_CH_ICOM), 0);
	/* saturation of the mix */
	all_up();
	up(0, ICOM_PTT_ICOM, 30000);
	up(1, ICOM_PTT_ICOM, 30000);
	up(2, ICOM_PTT_ICOM, -30000);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[3].pcm[ICOM_CH_ICOM][0], 30000);
	CHECK_EQ(out.down[2].pcm[ICOM_CH_ICOM][0], 32767);
	all_up();
	up(0, ICOM_PTT_ICOM, -30000);
	up(1, ICOM_PTT_ICOM, -30000);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[2].pcm[ICOM_CH_ICOM][0], -32768);

	/* CCU-03 alarm and radio receive channels with their flags */
	all_up();
	for (unsigned i = 0; i < ICOM_N; i++) {
		in.alarm[i] = 111;
		in.rx[0][i] = 222;
		in.rx[1][i] = 333;
	}
	in.alarm_on = true;
	in.rx_on = 0x2;
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[0].pcm[ICOM_CH_ALARM][0], 111);
	CHECK_EQ(out.down[0].pcm[ICOM_CH_RADIO1][0], 222);
	CHECK_EQ(out.down[0].pcm[ICOM_CH_RADIO2][0], 333);
	CHECK_EQ(out.down[0].flags, (1u << ICOM_CH_ALARM) | (1u << ICOM_CH_RADIO2));
	in.alarm_on = false;

	/* CCU-04 radio transmit: lowest ID wins, commander overrides */
	all_up();
	up(2, ICOM_PTT_R1, 500);
	up(3, ICOM_PTT_R1, 600);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[0], 2);
	CHECK_EQ(out.tx[0][0], 500);
	CHECK_EQ(out.tx_key, 0x1);
	CHECK_EQ(out.tx_owner[1], -1);
	CHECK_EQ(out.tx[1][0], 0);
	all_up();
	up(2, ICOM_PTT_R1, 500);
	up(0, ICOM_PTT_R1, 700); /* the commander (box 0) */
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[0], 0);
	icom_ccu_init(&c, 4, 3); /* commander last in the scan */
	all_up();
	up(1, ICOM_PTT_R2, 400);
	up(3, ICOM_PTT_R2, 800);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[1], 3);
	CHECK_EQ(out.tx[1][0], 800);

	/* a radio PTT from a box without audio (telemetry-only uplink) does not key it */
	all_up();
	{
		icom_up_t u;
		memset(&u, 0, sizeof(u));
		u.box_id = 2;
		u.ptt = ICOM_PTT_R1;
		u.nch = 0;
		icom_ccu_uplink(&c, &u);
	}
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[0], -1);

	/* CCU-05 rebroadcast: radio 2 rx -> radio 1 tx while radio 2 squelch is open */
	icom_ccu_init(&c, 4, 0);
	c.mode = ICOM_CCU_REBROADCAST;
	all_up();
	in.rx_on = 0x2;
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[0], -2);
	CHECK_EQ(out.tx[0][0], 333);
	CHECK_EQ(out.tx_key & 0x1, 0x1);
	CHECK_EQ(out.tx_key & 0x2, 0); /* radio 1 squelch closed: nothing to radio 2 */
	CHECK_EQ(out.down[0].ccu_mode, ICOM_CCU_REBROADCAST);
	/* a crew member keying radio 1 overrides the rebroadcast */
	all_up();
	up(1, ICOM_PTT_R1, 900);
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.tx_owner[0], 1);
	CHECK_EQ(out.tx[0][0], 900);
	in.rx_on = 0;

	/* CCU-06 commands are one-shot; out-of-range boxes are ignored */
	icom_ccu_command(&c, 2, ICOM_CMD_MAINT_ENTER, 7);
	icom_ccu_command(&c, 9, ICOM_CMD_MAINT_ENTER, 7);
	all_up();
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[2].cmd, ICOM_CMD_MAINT_ENTER);
	CHECK_EQ(out.down[2].arg, 7);
	CHECK_EQ(out.down[1].cmd, ICOM_CMD_NONE);
	all_up();
	icom_ccu_tick(&c, &in, &out);
	CHECK_EQ(out.down[2].cmd, ICOM_CMD_NONE);
	{
		icom_up_t u;
		memset(&u, 0, sizeof(u));
		u.box_id = 9;
		icom_ccu_uplink(&c, &u); /* ignored */
	}

	/* CCU-01 supervision: box 3 silent for 20 ticks is lost; back on its next uplink */
	uint8_t seq0 = c.seq;
	for (int t = 0; t < (int)ICOM_CCU_T_LOST_MS - 1; t++) {
		for (uint8_t b = 0; b < 3; b++) up(b, 0, 0);
		icom_ccu_tick(&c, &in, &out);
	}
	CHECK(!c.lost[3]);
	CHECK_EQ(out.lost, 0);
	for (uint8_t b = 0; b < 3; b++) up(b, 0, 0);
	icom_ccu_tick(&c, &in, &out);
	CHECK(c.lost[3]);
	CHECK_EQ(out.lost, 1);
	CHECK_EQ((uint8_t)(c.seq - seq0), ICOM_CCU_T_LOST_MS);
	all_up();
	icom_ccu_tick(&c, &in, &out);
	CHECK(!c.lost[3]);
	CHECK_EQ(c.box_state[3], ICOM_ST_NORMAL);
}
