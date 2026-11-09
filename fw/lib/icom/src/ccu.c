/*
 * Central communication unit core (CCU-01..CCU-08), DOSSIER §4 and §6.
 *
 * Every 1 ms tick:
 *  - supervision: a box without an uplink for ICOM_CCU_T_LOST_MS ticks is
 *    declared lost (its own link supervision puts it in BYPASS);
 *  - intercom: "mix-minus" - each box receives the sum of the other boxes
 *    that hold intercom PTT, never its own voice (its sidetone is local);
 *  - radio transmit: the commander's box wins a radio when it keys it,
 *    otherwise the lowest box ID that keys it;
 *  - REBROADCAST: with no crew member keying a radio, the other radio's
 *    receive audio is retransmitted on it while that radio's squelch is open;
 *  - downlink to each box: alarm, radio 1 rx, radio 2 rx, intercom mix, with
 *    activity flags, vehicle speed and one-shot commands.
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

void icom_ccu_init(icom_ccu_t *c, uint32_t n_box, uint8_t commander)
{
	uint32_t b;

	c->n_box = (n_box > ICOM_MAX_BOX) ? ICOM_MAX_BOX : n_box;
	c->commander = commander;
	c->mode = ICOM_CCU_NORMAL;
	c->seq = 0U;
	for (b = 0U; b < ICOM_MAX_BOX; b++) {
		c->have_up[b] = false;
		c->missed[b] = 0U;
		c->lost[b] = false;
		c->box_state[b] = 0U;
		c->cmd[b] = ICOM_CMD_NONE;
		c->arg[b] = 0;
	}
}

void icom_ccu_uplink(icom_ccu_t *c, const icom_up_t *u)
{
	if ((uint32_t)u->box_id < c->n_box) {
		c->up[u->box_id] = *u;
		c->have_up[u->box_id] = true;
	}
}

void icom_ccu_command(icom_ccu_t *c, uint8_t box, uint8_t cmd, int16_t arg)
{
	if ((uint32_t)box < c->n_box) {
		c->cmd[box] = cmd;
		c->arg[box] = arg;
	}
}

static int16_t sat16(int32_t v)
{
	int16_t r;

	if (v > 32767) {
		r = 32767;
	} else if (v < -32768) {
		r = -32768;
	} else {
		r = (int16_t)v;
	}
	return r;
}

/* the box that may use radio k (0/1), or -1 */
static int8_t radio_owner(const icom_ccu_t *c, uint8_t ptt_bit)
{
	int8_t owner = -1;
	uint32_t b;

	for (b = 0U; b < c->n_box; b++) {
		const bool keys = c->have_up[b] && ((c->up[b].ptt & ptt_bit) != 0U) && (c->up[b].nch == 1U);
		if (keys && ((owner < 0) || ((uint8_t)b == c->commander))) {
			owner = (int8_t)b;
		}
	}
	return owner;
}

static void supervise(icom_ccu_t *c, icom_ccu_out_t *out)
{
	uint32_t b;

	out->lost = 0U;
	for (b = 0U; b < c->n_box; b++) {
		if (c->have_up[b]) {
			c->missed[b] = 0U;
			c->lost[b] = false;
			c->box_state[b] = c->up[b].state;
		} else {
			c->missed[b]++;
			if (c->missed[b] >= ICOM_CCU_T_LOST_MS) {
				c->lost[b] = true;
			}
		}
		if (c->lost[b]) {
			out->lost++;
		}
	}
}

static void route_radios(const icom_ccu_t *c, const icom_ccu_in_t *in, icom_ccu_out_t *out)
{
	static const uint8_t k_ptt[2] = {ICOM_PTT_R1, ICOM_PTT_R2};
	uint32_t k;
	uint32_t i;

	out->tx_key = 0U;
	for (k = 0U; k < 2U; k++) {
		const int8_t owner = radio_owner(c, k_ptt[k]);
		const uint32_t other = 1U - k;
		out->tx_owner[k] = owner;
		if (owner >= 0) {
			for (i = 0U; i < ICOM_N; i++) {
				out->tx[k][i] = c->up[(uint32_t)owner].pcm[i];
			}
			out->tx_key = (uint8_t)(out->tx_key | (uint8_t)(1U << k));
		} else if ((c->mode == ICOM_CCU_REBROADCAST) && ((((uint32_t)in->rx_on >> other) & 1U) != 0U)) {
			for (i = 0U; i < ICOM_N; i++) {
				out->tx[k][i] = in->rx[other][i];
			}
			out->tx_owner[k] = -2;
			out->tx_key = (uint8_t)(out->tx_key | (uint8_t)(1U << k));
		} else {
			for (i = 0U; i < ICOM_N; i++) {
				out->tx[k][i] = 0;
			}
		}
	}
}

void icom_ccu_tick(icom_ccu_t *c, const icom_ccu_in_t *in, icom_ccu_out_t *out)
{
	int32_t sum[ICOM_N];
	bool talks[ICOM_MAX_BOX];
	uint32_t b;
	uint32_t i;
	uint32_t talkers = 0U;

	supervise(c, out);
	for (i = 0U; i < ICOM_N; i++) {
		sum[i] = 0;
	}
	for (b = 0U; b < c->n_box; b++) {
		talks[b] = c->have_up[b] && ((c->up[b].ptt & ICOM_PTT_ICOM) != 0U) && (c->up[b].nch == 1U);
		if (talks[b]) {
			talkers++;
			for (i = 0U; i < ICOM_N; i++) {
				sum[i] += (int32_t)c->up[b].pcm[i];
			}
		}
	}
	route_radios(c, in, out);

	for (b = 0U; b < c->n_box; b++) {
		icom_down_t *d = &out->down[b];
		uint8_t flags;
		const uint32_t others = talks[b] ? (talkers - 1U) : talkers;

		d->seq = c->seq;
		d->ccu_mode = c->mode;
		d->speed_ckmh = in->speed_ckmh;
		d->cmd = c->cmd[b];
		d->arg = c->arg[b];
		d->nch = (uint8_t)ICOM_NCH;
		c->cmd[b] = ICOM_CMD_NONE;
		c->arg[b] = 0;
		for (i = 0U; i < ICOM_N; i++) {
			const int32_t own = talks[b] ? (int32_t)c->up[b].pcm[i] : 0;
			d->pcm[ICOM_CH_ALARM][i] = in->alarm[i];
			d->pcm[ICOM_CH_RADIO1][i] = in->rx[0][i];
			d->pcm[ICOM_CH_RADIO2][i] = in->rx[1][i];
			d->pcm[ICOM_CH_ICOM][i] = sat16(sum[i] - own);
		}
		if (in->alarm_on) {
			flags = (uint8_t)(1U << ICOM_CH_ALARM);
		} else {
			flags = 0U;
		}
		flags = (uint8_t)(flags | (uint8_t)(((uint32_t)in->rx_on & 3U) << ICOM_CH_RADIO1));
		if (others > 0U) {
			flags = (uint8_t)(flags | (uint8_t)(1U << ICOM_CH_ICOM));
		}
		d->flags = flags;
	}
	for (b = 0U; b < ICOM_MAX_BOX; b++) {
		c->have_up[b] = false;
	}
	c->seq++;
}
