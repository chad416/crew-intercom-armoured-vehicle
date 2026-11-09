/*
 * Crew box top level: one call per 1 ms frame (CB-01..CB-05).
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

#define TELE_STATE_WHY 0U
#define TELE_ANR_ATT 1U
#define TELE_LOST 2U
#define TELE_N 4U

void icom_box_init(icom_box_t *b, uint8_t box_id)
{
	b->box_id = box_id;
	b->seq = 0U;
	icom_parser_init(&b->parser);
	icom_link_init(&b->link);
	icom_modes_init(&b->modes);
	icom_mix_init(&b->mix);
	icom_anr_init(&b->anr, 0.15f, 3U, 0.002f);
	icom_anr_set_path(&b->anr, 1.0f, 0.003f);
	b->rx_valid = false;
	b->bits = 0U;
	b->tele_rr = 0U;
	b->speed_cckmh = 0U;
	b->max_cycles = 0U;
}

void icom_box_rx(icom_box_t *b, const uint8_t *p, uint32_t n)
{
	uint32_t i;

	for (i = 0U; i < n; i++) {
		if (icom_parser_feed(&b->parser, p[i], &b->rx, &b->rx_up) == ICOM_RX_DOWN) {
			b->rx_valid = true;
		}
	}
}

static void command(icom_box_t *b, const icom_down_t *d)
{
	if (d->cmd == ICOM_CMD_ANR_RESET) {
		icom_anr_reset(&b->anr);
	} else if ((d->cmd == ICOM_CMD_SHAT_ROT) && (b->modes.state == ICOM_ST_MAINT)) {
		icom_anr_rotate(&b->anr, (float)d->arg);
	} else {
		/* other commands are handled by the mode machine */
	}
}

static uint16_t telemetry(icom_box_t *b, uint8_t id)
{
	uint16_t v;

	if (id == TELE_STATE_WHY) {
		v = (uint16_t)((uint16_t)((uint16_t)b->modes.why << 8U) | (uint16_t)b->modes.transitions);
	} else if (id == TELE_ANR_ATT) {
		v = icom_anr_atten_x10(&b->anr);
	} else if (id == TELE_LOST) {
		v = (uint16_t)b->link.lost_total;
	} else {
		/* item 3: CRC errors */
		v = (uint16_t)b->parser.crc_errors;
	}
	return v;
}

uint32_t icom_box_tick(icom_box_t *b, const int16_t mic[ICOM_N], const int16_t err[ICOM_N], uint8_t bits,
		       int16_t out[ICOM_N], uint8_t *upbuf, uint32_t cap)
{
	icom_chans_t pcm;
	float errf[ICOM_N];
	float anti[ICOM_N];
	icom_mode_in_t in;
	icom_state_t st;
	uint8_t flags;
	uint32_t i;
	uint32_t len = 0U;
	const icom_down_t *frame = b->rx_valid ? &b->rx : NULL;

	flags = icom_link_tick(&b->link, frame, &pcm);
	in.cmd = ICOM_CMD_NONE;
	if (frame != NULL) {
		b->speed_cckmh = frame->speed_ckmh;
		in.cmd = frame->cmd;
		command(b, frame);
	}
	b->rx_valid = false;

	b->bits = (uint8_t)((uint8_t)(bits & (uint8_t)~ICOM_BIT_ANR) | (b->anr.fault ? ICOM_BIT_ANR : 0U));
	in.link_ok = (frame != NULL);
	in.missed_run = b->link.missed_run;
	in.good_run = b->link.good_run;
	in.bits = b->bits;
	in.speed_ckmh = b->speed_cckmh;
	st = icom_modes_tick(&b->modes, &in);

	for (i = 0U; i < ICOM_N; i++) {
		errf[i] = (float)err[i] / 32768.0f;
	}
	if ((st == ICOM_ST_NORMAL) || (st == ICOM_ST_MAINT)) {
		icom_anr_frame(&b->anr, errf, (float)b->speed_cckmh / 100.0f, anti);
	} else {
		/* ANR off in DEGRADED, BYPASS and INIT: speed 0 disables and clears it */
		icom_anr_frame(&b->anr, errf, 0.0f, anti);
	}
	(void)icom_mix_frame(&b->mix, &pcm, flags, mic, anti, out);

	if (st != ICOM_ST_INIT) {
		b->up.seq = b->seq;
		b->up.box_id = b->box_id;
		b->up.state = (uint8_t)st;
		b->up.bits = b->bits;
		b->up.ptt = 0U;
		b->up.tele_id = b->tele_rr;
		b->up.tele = telemetry(b, b->tele_rr);
		if (st == ICOM_ST_BYPASS) {
			b->up.nch = 0U;
		} else {
			b->up.nch = 1U;
		}
		for (i = 0U; i < ICOM_N; i++) {
			b->up.pcm[i] = mic[i];
		}
		len = icom_up_encode(&b->up, upbuf, cap);
		b->seq++;
		b->tele_rr = (uint8_t)((b->tele_rr + 1U) % TELE_N);
	}
	return len;
}
