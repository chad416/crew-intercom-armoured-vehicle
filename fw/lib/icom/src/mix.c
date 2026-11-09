/*
 * Priority mixer, local sidetone and output limiter (CB-30..CB-35).
 *
 * Priority (rank 0 = highest): ALARM 0, intercom 1, radio 1 and radio 2 both 2.
 * A channel is "active" while its flag is set and its frame mean square is
 * above act_thr, and for hold_ms after that. Every active channel whose rank
 * is lower than that of another active channel is multiplied by duck.
 * Sidetone is the local microphone times the sidetone gain: it never goes
 * over the bus, so its delay is only the codec and one frame (DOSSIER §7).
 * The audio sum is peak-limited per frame to limit (instant attack, ~100 ms
 * release); the anti-noise, already clamped by the ANR, is added after the
 * limiter so that the limiter gain does not modulate the ANR loop.
 * SPDX-License-Identifier: MIT
 */
#include "icom/icom.h"

void icom_mix_init(icom_mix_t *x)
{
	uint32_t c;

	for (c = 0U; c < ICOM_NCH; c++) {
		x->vol[c] = 1.0f;
		x->hold[c] = 0U;
	}
	x->sidetone = 0.25f;
	x->duck = 0.25f;
	x->limit = 0.5f;
	x->env = 1.0f;
	x->act_thr = 1.0e-5f;
	x->hold_ms = 300U;
}

static float to_f(int16_t s)
{
	return (float)s / 32768.0f;
}

static int16_t to_s16(float v)
{
	float s = v * 32768.0f;
	int16_t r;

	if (s > 32767.0f) {
		r = 32767;
	} else if (s < -32768.0f) {
		r = -32768;
	} else {
		r = (int16_t)s;
	}
	return r;
}

uint8_t icom_mix_frame(icom_mix_t *x, const icom_chans_t *pcm, uint8_t flags,
		       const int16_t mic[ICOM_N], const float anti[ICOM_N], int16_t out[ICOM_N])
{
	static const uint8_t k_rank[ICOM_NCH] = {0U, 2U, 2U, 1U};
	uint8_t active = 0U;
	uint8_t best = 0xFFU;
	float g[ICOM_NCH];
	float sum[ICOM_N];
	float peak = 0.0f;
	uint32_t c;
	uint32_t i;

	for (c = 0U; c < ICOM_NCH; c++) {
		float ms = 0.0f;
		for (i = 0U; i < ICOM_N; i++) {
			const float v = to_f(pcm->s[c][i]);
			ms += v * v;
		}
		ms /= (float)ICOM_N;
		if ((((uint32_t)flags >> c) & 1U) != 0U) {
			if (ms > x->act_thr) {
				x->hold[c] = x->hold_ms;
			}
		} else {
			x->hold[c] = 0U;
		}
		if (x->hold[c] > 0U) {
			x->hold[c]--;
			active = (uint8_t)(active | (uint8_t)(1U << c));
			if (k_rank[c] < best) {
				best = k_rank[c];
			}
		}
	}
	for (c = 0U; c < ICOM_NCH; c++) {
		g[c] = x->vol[c];
		if ((((uint32_t)active >> c) & 1U) == 0U) {
			g[c] = 0.0f;
		} else if (k_rank[c] > best) {
			g[c] *= x->duck;
		} else {
			/* highest active priority: full volume */
		}
	}
	for (i = 0U; i < ICOM_N; i++) {
		float s = x->sidetone * to_f(mic[i]);
		for (c = 0U; c < ICOM_NCH; c++) {
			s += g[c] * to_f(pcm->s[c][i]);
		}
		sum[i] = s;
		if (s > peak) {
			peak = s;
		} else if (-s > peak) {
			peak = -s;
		} else {
			/* not a new peak */
		}
	}
	/* limiter: release towards 1, then instant attack for this frame */
	x->env += (1.0f - x->env) * 0.01f;
	if ((peak * x->env) > x->limit) {
		x->env = x->limit / peak;
	}
	for (i = 0U; i < ICOM_N; i++) {
		float v = sum[i] * x->env;
		if (anti != NULL) {
			v += anti[i];
		}
		out[i] = to_s16(v);
	}
	return active;
}
