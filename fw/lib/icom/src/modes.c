/*
 * Crew-box operating states and transitions (CB-20..CB-29), DOSSIER §5.
 *
 *   INIT     power-up after POST; digital path off, relay de-energised (bypass)
 *   NORMAL   digital path through the CCU, ANR on
 *   DEGRADED digital path, a non-critical function lost (ANR off)
 *   BYPASS   relay de-energised: headset on the analog emergency line
 *   MAINT    maintenance, entered only at standstill on a CCU command
 *
 * Transition table (exit criteria of the current state):
 *   INIT     -> NORMAL    link up and no critical BIT fault
 *   INIT     -> BYPASS    critical BIT fault (latched) or no link within T_INIT
 *   NORMAL   -> DEGRADED  ANR BIT fault, or emergency-path self-test failed
 *   NORMAL/DEGRADED/MAINT -> BYPASS  no valid frame for T_LOSS, or critical fault (latched)
 *   DEGRADED -> NORMAL    ANR fault cleared
 *   BYPASS   -> NORMAL    not latched, link good for T_RECOVER (DEGRADED if ANR fault)
 *   NORMAL/DEGRADED -> MAINT  MAINT_ENTER command at speed 0
 *   MAINT    -> NORMAL    MAINT_EXIT command, speed > 0, or T_MAINT timeout
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

void icom_modes_init(icom_modes_t *m)
{
	m->state = ICOM_ST_INIT;
	m->why = ICOM_WHY_NONE;
	m->t_in_state = 0U;
	m->latched = false;
	m->relay_phase = false;
	m->transitions = 0U;
}

static void go(icom_modes_t *m, icom_state_t s, icom_why_t why)
{
	m->state = s;
	m->why = why;
	m->t_in_state = 0U;
	m->transitions++;
}

static icom_state_t digital_state(uint8_t bits)
{
	icom_state_t s = ICOM_ST_NORMAL;

	if ((bits & (uint8_t)(ICOM_BIT_ANR | ICOM_BIT_BYPASS)) != 0U) {
		s = ICOM_ST_DEGRADED;
	}
	return s;
}

static void tick_init(icom_modes_t *m, const icom_mode_in_t *in)
{
	if ((in->bits & ICOM_BIT_CRITICAL) != 0U) {
		m->latched = true;
		go(m, ICOM_ST_BYPASS, ICOM_WHY_CRITICAL);
	} else if (in->link_ok) {
		go(m, digital_state(in->bits), ICOM_WHY_NONE);
	} else if (m->t_in_state >= ICOM_T_INIT_MS) {
		go(m, ICOM_ST_BYPASS, ICOM_WHY_NO_LINK_AT_INIT);
	} else {
		/* wait for the link */
	}
}

static void tick_digital(icom_modes_t *m, const icom_mode_in_t *in)
{
	if ((in->bits & ICOM_BIT_CRITICAL) != 0U) {
		m->latched = true;
		go(m, ICOM_ST_BYPASS, ICOM_WHY_CRITICAL);
	} else if (in->missed_run >= ICOM_T_LOSS_MS) {
		go(m, ICOM_ST_BYPASS, ICOM_WHY_LINK);
	} else if (m->state == ICOM_ST_MAINT) {
		if ((in->cmd == ICOM_CMD_MAINT_EXIT) || (in->speed_ckmh != 0U) ||
		    (m->t_in_state >= ICOM_T_MAINT_MS)) {
			go(m, digital_state(in->bits), ICOM_WHY_NONE);
		}
	} else if ((in->cmd == ICOM_CMD_MAINT_ENTER) && (in->speed_ckmh == 0U)) {
		go(m, ICOM_ST_MAINT, ICOM_WHY_NONE);
	} else if (digital_state(in->bits) != m->state) {
		go(m, digital_state(in->bits), ICOM_WHY_NONE);
	} else {
		/* stay */
	}
}

static void tick_bypass(icom_modes_t *m, const icom_mode_in_t *in)
{
	if ((in->bits & ICOM_BIT_CRITICAL) != 0U) {
		m->latched = true;
		m->why = ICOM_WHY_CRITICAL;
	} else if ((!m->latched) && (in->good_run >= ICOM_T_RECOVER_MS)) {
		go(m, digital_state(in->bits), ICOM_WHY_NONE);
	} else {
		/* stay in bypass */
	}
}

icom_state_t icom_modes_tick(icom_modes_t *m, const icom_mode_in_t *in)
{
	const icom_state_t before = m->state;

	m->t_in_state++;
	if (before == ICOM_ST_INIT) {
		tick_init(m, in);
	} else if (before == ICOM_ST_BYPASS) {
		tick_bypass(m, in);
	} else {
		tick_digital(m, in);
	}
	return m->state;
}

bool icom_modes_relay_drive(icom_modes_t *m)
{
	if ((m->state == ICOM_ST_INIT) || (m->state == ICOM_ST_BYPASS)) {
		m->relay_phase = false;
	} else {
		m->relay_phase = !m->relay_phase;
	}
	return m->relay_phase;
}
