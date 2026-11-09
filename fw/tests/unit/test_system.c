/*
 * Host system integration (DOSSIER §13.2, integration point IP3 in software):
 * four crew boxes and the CCU core exchange real encoded bus frames every
 * 1 ms tick. Measures the digital pipeline delay crew-to-crew and checks the
 * fail-operational scenarios end to end (SYS-3.2.5, SYS-3.2.7).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

#define NB 4

static icom_box_t boxes[NB];
static icom_ccu_t ccu;
static icom_ccu_in_t cin;
static icom_ccu_out_t cout;
static int16_t mic[NB][ICOM_N], outp[NB][ICOM_N];
static const int16_t zero[ICOM_N];
static int box_dead[NB];
static int ccu_dead;
static icom_parser_t ccu_parser;

/* one system tick: boxes consume last tick's downlinks and send uplinks; the
 * CCU receives those, mixes and sends the downlinks for the next tick */
static void sys_tick(void)
{
	uint8_t buf[ICOM_MAX_FRAME];
	for (int b = 0; b < NB; b++) {
		if (box_dead[b]) continue;
		uint32_t n = icom_box_tick(&boxes[b], mic[b], zero, zero, 0, outp[b], buf, sizeof(buf));
		if (!ccu_dead) {
			icom_down_t dd;
			icom_up_t u;
			for (uint32_t i = 0; i < n; i++)
				if (icom_parser_feed(&ccu_parser, buf[i], &dd, &u) == ICOM_RX_UP) icom_ccu_uplink(&ccu, &u);
		}
	}
	if (ccu_dead) return;
	icom_ccu_tick(&ccu, &cin, &cout);
	for (int b = 0; b < NB; b++) {
		uint32_t n = icom_down_encode(&cout.down[b], buf, sizeof(buf));
		if (!box_dead[b]) icom_box_rx(&boxes[b], buf, n);
	}
}

static void setup(void)
{
	memset(&cin, 0, sizeof(cin));
	memset(mic, 0, sizeof(mic));
	memset(box_dead, 0, sizeof(box_dead));
	ccu_dead = 0;
	icom_ccu_init(&ccu, NB, 0);
	icom_parser_init(&ccu_parser);
	for (int b = 0; b < NB; b++) {
		icom_box_init(&boxes[b], (uint8_t)b);
		boxes[b].mix.sidetone = 0.0f; /* measure the bus path alone */
	}
	for (int t = 0; t < 5; t++) sys_tick();
}

void test_system(void)
{
	setup();
	for (int b = 0; b < NB; b++) CHECK_EQ(boxes[b].modes.state, ICOM_ST_NORMAL);

	/* crew-to-crew delay: box 0 keys intercom and speaks at tick T; ticks until box 1 hears it */
	boxes[0].ptt = ICOM_PTT_ICOM;
	int heard = -1;
	for (int t = 0; t < 20 && heard < 0; t++) {
		for (unsigned i = 0; i < ICOM_N; i++) mic[0][i] = (int16_t)((i & 1u) ? -4000 : 4000);
		sys_tick();
		if (outp[1][0] != 0) heard = t;
	}
	printf("  system: crew-to-crew digital pipeline %d frame(s) after the speech frame\n", heard);
	CHECK(heard >= 0 && heard <= 2);
	CHECK_EQ(outp[0][0], 0); /* mix-minus: the talker does not hear itself over the bus */
	CHECK(outp[2][0] != 0);
	boxes[0].ptt = 0;
	memset(mic, 0, sizeof(mic));

	/* SYS-3.2.7: box 2 dies -> the CCU declares it lost; the others keep talking */
	box_dead[2] = 1;
	for (int t = 0; t < (int)ICOM_CCU_T_LOST_MS; t++) sys_tick();
	CHECK(ccu.lost[2]);
	CHECK_EQ(cout.lost, 1);
	for (int b = 0; b < NB; b++)
		if (b != 2) CHECK_EQ(boxes[b].modes.state, ICOM_ST_NORMAL);
	boxes[3].ptt = ICOM_PTT_ICOM;
	for (unsigned i = 0; i < ICOM_N; i++) mic[3][i] = 3000;
	sys_tick();
	sys_tick();
	CHECK(outp[0][0] != 0);
	boxes[3].ptt = 0;
	memset(mic, 0, sizeof(mic));

	/* SYS-3.2.7: the CCU dies -> every live box is in BYPASS after 20 ticks */
	ccu_dead = 1;
	int t_bypass = -1;
	for (int t = 0; t < 40; t++) {
		sys_tick();
		int all = 1;
		for (int b = 0; b < NB; b++)
			if (!box_dead[b] && boxes[b].modes.state != ICOM_ST_BYPASS) all = 0;
		if (all && t_bypass < 0) t_bypass = t + 1;
	}
	printf("  system: all live boxes in BYPASS %d ms after the CCU died\n", t_bypass);
	/* the CCU's last downlink arrives one tick after it was built, so BYPASS comes
	 * T_LOSS ticks after the last frame received = T_LOSS + 1 after the CCU died */
	CHECK_EQ(t_bypass, (int)ICOM_T_LOSS_MS + 1);

	/* CCU back: boxes return to NORMAL after 2 s of good frames, all together */
	ccu_dead = 0;
	icom_ccu_init(&ccu, NB, 0);
	for (int t = 0; t < (int)ICOM_T_RECOVER_MS + 2; t++) sys_tick();
	for (int b = 0; b < NB; b++)
		if (!box_dead[b]) CHECK_EQ(boxes[b].modes.state, ICOM_ST_NORMAL);

	/* radio: box 1 keys radio 2; the commander (box 0) keys it too and wins */
	box_dead[2] = 0;
	boxes[1].ptt = ICOM_PTT_R2;
	for (unsigned i = 0; i < ICOM_N; i++) mic[1][i] = 1234;
	sys_tick();
	CHECK_EQ(cout.tx_owner[1], 1);
	CHECK_EQ(cout.tx[1][0], 1234);
	boxes[0].ptt = ICOM_PTT_R2;
	for (unsigned i = 0; i < ICOM_N; i++) mic[0][i] = 2345;
	sys_tick();
	CHECK_EQ(cout.tx_owner[1], 0);
	CHECK_EQ(cout.tx[1][0], 2345);
}
