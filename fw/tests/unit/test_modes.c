/*
 * Operating states and transitions (CB-20..CB-29).
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

static icom_mode_in_t in_link(int ok, unsigned good, unsigned missed, uint8_t bits)
{
	icom_mode_in_t in;
	memset(&in, 0, sizeof(in));
	in.link_ok = ok;
	in.good_run = good;
	in.missed_run = missed;
	in.bits = bits;
	return in;
}

static void run_ticks(icom_modes_t *m, icom_mode_in_t in, unsigned n)
{
	for (unsigned i = 0; i < n; i++) icom_modes_tick(m, &in);
}

void test_modes(void)
{
	icom_modes_t m;
	icom_mode_in_t in;

	/* INIT: relay off, waits for the link, then NORMAL */
	icom_modes_init(&m);
	CHECK_EQ(icom_modes_relay_drive(&m), 0);
	in = in_link(0, 0, 1, 0);
	run_ticks(&m, in, 100);
	CHECK_EQ(m.state, ICOM_ST_INIT);
	in = in_link(1, 1, 0, 0);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	/* relay drive toggles while the digital path is on (CB-27) */
	bool a = icom_modes_relay_drive(&m);
	bool b = icom_modes_relay_drive(&m);
	CHECK(a != b);

	/* INIT with ANR fault -> DEGRADED */
	icom_modes_init(&m);
	in = in_link(1, 1, 0, ICOM_BIT_ANR);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_DEGRADED);

	/* INIT, no link for T_INIT -> BYPASS (not latched) */
	icom_modes_init(&m);
	in = in_link(0, 0, 5, 0);
	run_ticks(&m, in, ICOM_T_INIT_MS - 1);
	CHECK_EQ(m.state, ICOM_ST_INIT);
	icom_modes_tick(&m, &in);
	CHECK_EQ(m.state, ICOM_ST_BYPASS);
	CHECK_EQ(m.why, ICOM_WHY_NO_LINK_AT_INIT);
	CHECK(!m.latched);
	CHECK_EQ(icom_modes_relay_drive(&m), 0);

	/* INIT with critical BIT -> latched BYPASS (CB-24) */
	icom_modes_init(&m);
	in = in_link(1, 1, 0, ICOM_BIT_CODEC);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_BYPASS);
	CHECK(m.latched);
	in = in_link(1, 5000, 0, 0);
	run_ticks(&m, in, 10);
	CHECK_EQ(m.state, ICOM_ST_BYPASS); /* latched: no recovery */

	/* NORMAL -> BYPASS after T_LOSS ticks without a frame (CB-22) */
	icom_modes_init(&m);
	in = in_link(1, 1, 0, 0);
	icom_modes_tick(&m, &in);
	in = in_link(0, 0, ICOM_T_LOSS_MS - 1, 0);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	in.missed_run = ICOM_T_LOSS_MS;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_BYPASS);
	CHECK_EQ(m.why, ICOM_WHY_LINK);

	/* BYPASS -> back only after T_RECOVER continuous good frames (CB-23) */
	in = in_link(1, ICOM_T_RECOVER_MS - 1, 0, 0);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_BYPASS);
	in.good_run = ICOM_T_RECOVER_MS;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	/* ... to DEGRADED if the ANR is faulty */
	in = in_link(0, 0, ICOM_T_LOSS_MS, 0);
	icom_modes_tick(&m, &in);
	in = in_link(1, ICOM_T_RECOVER_MS, 0, ICOM_BIT_ANR);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_DEGRADED);

	/* DEGRADED <-> NORMAL follows the ANR bit (CB-25) */
	in = in_link(1, 1, 0, 0);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	in.bits = ICOM_BIT_ANR;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_DEGRADED);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_DEGRADED);

	/* critical fault in a digital state -> latched BYPASS */
	in.bits = ICOM_BIT_SUPPLY;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_BYPASS);
	CHECK(m.latched);
	/* a critical fault that appears while already in (link) BYPASS latches too */
	icom_modes_init(&m);
	in = in_link(1, 1, 0, 0);
	icom_modes_tick(&m, &in);
	in = in_link(0, 0, ICOM_T_LOSS_MS, 0);
	icom_modes_tick(&m, &in);
	CHECK(!m.latched);
	in = in_link(0, 0, ICOM_T_LOSS_MS, ICOM_BIT_CODEC);
	icom_modes_tick(&m, &in);
	CHECK(m.latched);
	CHECK_EQ(m.why, ICOM_WHY_CRITICAL);

	/* MAINT only at standstill (CB-28) */
	icom_modes_init(&m);
	in = in_link(1, 1, 0, 0);
	icom_modes_tick(&m, &in);
	in.cmd = ICOM_CMD_MAINT_ENTER;
	in.speed_ckmh = 300;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	in.speed_ckmh = 0;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_MAINT);
	in.cmd = ICOM_CMD_NONE;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_MAINT);
	a = icom_modes_relay_drive(&m);
	b = icom_modes_relay_drive(&m);
	CHECK(a != b);
	in.cmd = ICOM_CMD_MAINT_EXIT;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	/* exit when the vehicle moves */
	in.cmd = ICOM_CMD_MAINT_ENTER;
	icom_modes_tick(&m, &in);
	in.cmd = ICOM_CMD_NONE;
	in.speed_ckmh = 1;
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_NORMAL);
	/* exit on timeout */
	in.speed_ckmh = 0;
	in.cmd = ICOM_CMD_MAINT_ENTER;
	icom_modes_tick(&m, &in);
	in.cmd = ICOM_CMD_NONE;
	run_ticks(&m, in, ICOM_T_MAINT_MS - 1);
	CHECK_EQ(m.state, ICOM_ST_MAINT);
	icom_modes_tick(&m, &in);
	CHECK_EQ(m.state, ICOM_ST_NORMAL);
	/* link loss in MAINT -> BYPASS */
	in.cmd = ICOM_CMD_MAINT_ENTER;
	icom_modes_tick(&m, &in);
	CHECK_EQ(m.state, ICOM_ST_MAINT);
	in = in_link(0, 0, ICOM_T_LOSS_MS, 0);
	CHECK_EQ(icom_modes_tick(&m, &in), ICOM_ST_BYPASS);
	CHECK(m.transitions > 0);
}
