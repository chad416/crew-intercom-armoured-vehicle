/*
 * Bus frames: CRC-16/CCITT-FALSE, encoders, byte-wise parser (CB-10..CB-13).
 *
 * Frame: 0xA5 0x5A len type payload[len] crc_hi crc_lo
 * The CRC covers len, type and payload. A frame with a bad CRC, an unknown
 * type or an inconsistent length is dropped and counted; the parser then
 * searches for the next sync pair, so one corrupted frame costs one frame.
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

uint16_t icom_crc16(const uint8_t *p, uint32_t n)
{
	uint16_t crc = 0xFFFFU;
	uint32_t i;
	uint32_t b;

	for (i = 0U; i < n; i++) {
		crc = (uint16_t)(crc ^ (uint16_t)((uint16_t)p[i] << 8U));
		for (b = 0U; b < 8U; b++) {
			if ((crc & 0x8000U) != 0U) {
				crc = (uint16_t)((uint16_t)(crc << 1U) ^ 0x1021U);
			} else {
				crc = (uint16_t)(crc << 1U);
			}
		}
	}
	return crc;
}

static void put16(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)(v >> 8U);
	p[1] = (uint8_t)(v & 0xFFU);
}

static uint16_t get16(const uint8_t *p)
{
	return (uint16_t)((uint16_t)((uint16_t)p[0] << 8U) | (uint16_t)p[1]);
}

static uint32_t finish(uint8_t *buf, uint32_t plen, uint8_t type)
{
	uint16_t crc;

	buf[0] = ICOM_SYNC0;
	buf[1] = ICOM_SYNC1;
	buf[2] = (uint8_t)plen;
	buf[3] = type;
	crc = icom_crc16(&buf[2], plen + 2U);
	put16(&buf[ICOM_HDR_LEN + plen], crc);
	return ICOM_HDR_LEN + plen + ICOM_CRC_LEN;
}

static void put_pcm(uint8_t *p, const int16_t *s)
{
	uint32_t i;

	for (i = 0U; i < ICOM_N; i++) {
		put16(&p[2U * i], (uint16_t)s[i]);
	}
}

static void get_pcm(const uint8_t *p, int16_t *s)
{
	uint32_t i;

	for (i = 0U; i < ICOM_N; i++) {
		s[i] = (int16_t)get16(&p[2U * i]);
	}
}

uint32_t icom_down_encode(const icom_down_t *d, uint8_t *buf, uint32_t cap)
{
	uint32_t len = 0U;
	uint8_t *p = &buf[ICOM_HDR_LEN];

	if (d->nch <= ICOM_NCH) {
		const uint32_t plen = ICOM_DOWN_FIX + ((uint32_t)d->nch * ICOM_N * 2U);
		if (cap >= (ICOM_HDR_LEN + plen + ICOM_CRC_LEN)) {
			p[0] = d->seq;
			p[1] = d->ccu_mode;
			p[2] = d->flags;
			put16(&p[3], d->speed_ckmh);
			p[5] = d->cmd;
			put16(&p[6], (uint16_t)d->arg);
			p[8] = d->nch;
			for (uint32_t c = 0U; c < (uint32_t)d->nch; c++) {
				put_pcm(&p[ICOM_DOWN_FIX + (c * ICOM_N * 2U)], d->pcm[c]);
			}
			len = finish(buf, plen, ICOM_T_DOWN);
		}
	}
	return len;
}

uint32_t icom_up_encode(const icom_up_t *u, uint8_t *buf, uint32_t cap)
{
	uint32_t len = 0U;
	uint8_t *p = &buf[ICOM_HDR_LEN];

	if (u->nch <= 1U) {
		const uint32_t plen = ICOM_UP_FIX + ((uint32_t)u->nch * ICOM_N * 2U);
		if (cap >= (ICOM_HDR_LEN + plen + ICOM_CRC_LEN)) {
			p[0] = u->seq;
			p[1] = u->box_id;
			p[2] = u->state;
			p[3] = u->bits;
			p[4] = u->ptt;
			p[5] = u->tele_id;
			put16(&p[6], u->tele);
			p[8] = u->nch;
			if (u->nch == 1U) {
				put_pcm(&p[ICOM_UP_FIX], u->pcm);
			}
			len = finish(buf, plen, ICOM_T_UP);
		}
	}
	return len;
}

void icom_parser_init(icom_parser_t *p)
{
	p->pos = 0U;
	p->need = 0U;
	p->crc_errors = 0U;
	p->format_errors = 0U;
	p->frames = 0U;
}

static icom_rx_kind_t decode(icom_parser_t *p, icom_down_t *down, icom_up_t *up)
{
	icom_rx_kind_t kind = ICOM_RX_NONE;
	const uint32_t plen = (uint32_t)p->buf[2];
	const uint8_t *q = &p->buf[ICOM_HDR_LEN];
	uint32_t c;

	if (get16(&q[plen]) != icom_crc16(&p->buf[2], plen + 2U)) {
		p->crc_errors++;
	} else if ((p->buf[3] == ICOM_T_DOWN) && (plen >= ICOM_DOWN_FIX) &&
		   (q[8] <= ICOM_NCH) && (plen == (ICOM_DOWN_FIX + ((uint32_t)q[8] * ICOM_N * 2U)))) {
		down->seq = q[0];
		down->ccu_mode = q[1];
		down->flags = q[2];
		down->speed_ckmh = get16(&q[3]);
		down->cmd = q[5];
		down->arg = (int16_t)get16(&q[6]);
		down->nch = q[8];
		for (c = 0U; c < (uint32_t)down->nch; c++) {
			get_pcm(&q[ICOM_DOWN_FIX + (c * ICOM_N * 2U)], down->pcm[c]);
		}
		kind = ICOM_RX_DOWN;
	} else if ((p->buf[3] == ICOM_T_UP) && (plen >= ICOM_UP_FIX) && (q[8] <= 1U) &&
		   (plen == (ICOM_UP_FIX + ((uint32_t)q[8] * ICOM_N * 2U)))) {
		up->seq = q[0];
		up->box_id = q[1];
		up->state = q[2];
		up->bits = q[3];
		up->ptt = q[4];
		up->tele_id = q[5];
		up->tele = get16(&q[6]);
		up->nch = q[8];
		if (up->nch == 1U) {
			get_pcm(&q[ICOM_UP_FIX], up->pcm);
		}
		kind = ICOM_RX_UP;
	} else {
		p->format_errors++;
	}
	if (kind != ICOM_RX_NONE) {
		p->frames++;
	}
	return kind;
}

icom_rx_kind_t icom_parser_feed(icom_parser_t *p, uint8_t byte, icom_down_t *down, icom_up_t *up)
{
	icom_rx_kind_t kind = ICOM_RX_NONE;

	if (p->pos == 0U) {
		if (byte == ICOM_SYNC0) {
			p->buf[0] = byte;
			p->pos = 1U;
		}
	} else if (p->pos == 1U) {
		if (byte == ICOM_SYNC1) {
			p->buf[1] = byte;
			p->pos = 2U;
		} else if (byte != ICOM_SYNC0) {
			p->pos = 0U;
		} else {
			/* A5 A5 5A: the second A5 may start the frame */
		}
	} else if (p->pos == 2U) {
		if ((uint32_t)byte > ICOM_MAX_PAYLOAD) {
			p->format_errors++;
			p->pos = 0U;
		} else {
			p->buf[2] = byte;
			p->need = ICOM_HDR_LEN + (uint32_t)byte + ICOM_CRC_LEN;
			p->pos = 3U;
		}
	} else {
		p->buf[p->pos] = byte;
		p->pos++;
		if (p->pos == p->need) {
			kind = decode(p, down, up);
			p->pos = 0U;
		}
	}
	return kind;
}
