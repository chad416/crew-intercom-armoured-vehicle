/*
 * Link supervision and packet-loss concealment (CB-14..CB-16).
 *
 * One downlink frame is expected per 1 ms tick. A missing frame (or a gap in
 * the sequence numbers) is concealed by repeating the last frame at half the
 * amplitude per repetition, for up to ICOM_PLC_FRAMES frames; after that the
 * audio is muted. The counters feed the mode state machine (bypass after
 * ICOM_T_LOSS_MS ticks without a valid frame).
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

void icom_link_init(icom_link_t *l)
{
	uint32_t c;
	uint32_t i;

	l->have_last = false;
	l->last_seq = 0U;
	l->missed_run = 0U;
	l->good_run = 0U;
	l->lost_total = 0U;
	l->concealed = 0U;
	l->last_flags = 0U;
	for (c = 0U; c < ICOM_NCH; c++) {
		for (i = 0U; i < ICOM_N; i++) {
			l->last_pcm[c][i] = 0;
		}
	}
}

uint8_t icom_link_tick(icom_link_t *l, const icom_down_t *frame, icom_chans_t *pcm)
{
	uint32_t c;
	uint32_t i;
	uint8_t flags;

	if (frame != NULL) {
		if (l->have_last) {
			/* frames skipped by the sender side count as lost */
			const uint8_t gap = (uint8_t)(frame->seq - (uint8_t)(l->last_seq + 1U));
			l->lost_total += (uint32_t)gap;
		}
		l->have_last = true;
		l->last_seq = frame->seq;
		l->missed_run = 0U;
		l->good_run++;
		for (c = 0U; c < ICOM_NCH; c++) {
			for (i = 0U; i < ICOM_N; i++) {
				const int16_t s = (c < (uint32_t)frame->nch) ? frame->pcm[c][i] : (int16_t)0;
				l->last_pcm[c][i] = s;
				pcm->s[c][i] = s;
			}
		}
		l->last_flags = frame->flags;
		flags = frame->flags;
	} else {
		l->missed_run++;
		l->good_run = 0U;
		l->lost_total++;
		if (l->missed_run <= ICOM_PLC_FRAMES) {
			l->concealed++;
			flags = l->last_flags;
		} else {
			flags = 0U;
		}
		for (c = 0U; c < ICOM_NCH; c++) {
			for (i = 0U; i < ICOM_N; i++) {
				/* halve on every repetition: -6 dB per concealed frame */
				l->last_pcm[c][i] = (int16_t)(l->last_pcm[c][i] / 2);
				pcm->s[c][i] = (flags != 0U) ? l->last_pcm[c][i] : (int16_t)0;
			}
		}
	}
	return flags;
}
