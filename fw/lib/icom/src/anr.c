/*
 * Speed-referenced harmonic FxLMS active noise reduction (CB-40..CB-47).
 *
 * f0 = v / pitch (track-laying frequency). For each harmonic h <= n_harm with
 * h*f0 in [f_min, f_max] the controller keeps two weights (cos, sin) and
 * outputs y = sum_h w_h . [cos(h*phi), sin(h*phi)]. The filtered reference is
 * the reference rotated and scaled by S_hat(h*f0), the end-to-end secondary
 * path (DAC, earphone, cup, error microphone, ADC and the frame pipeline)
 * measured at commissioning. Normalised update:
 *     w_h <- w_h - mu * e * S_hat x_h / |S_hat|^2
 * Stable while the phase error of S_hat is within +/-90 deg; the guard below
 * detects the divergent case and switches the ANR off (DEGRADED, CB-46).
 * SPDX-License-Identifier: MIT
 */
#include <math.h>

#include "icom/icom.h"

#define TWO_PI 6.28318530717958647692f
#define MS_ALPHA (1.0f / 1600.0f)        /* 100 ms at 16 kHz */
#define GUARD_SETTLE (ICOM_FS_HZ / 2U)    /* 0.5 s after enable */
#define GUARD_TRIP (ICOM_FS_HZ / 5U)      /* 0.2 s above the limit */
#define GUARD_RATIO 4.0f                 /* +6 dB over the error before ANR */
#define WARMUP (ICOM_FS_HZ / 5U)          /* 0.2 s of error observed before enable */

void icom_anr_init(icom_anr_t *a, float pitch_m, uint32_t n_harm, float mu)
{
	uint32_t k;

	a->pitch_m = pitch_m;
	a->n_harm = (n_harm > ICOM_ANR_HMAX) ? ICOM_ANR_HMAX : n_harm;
	a->mu = mu;
	a->f_min_hz = 20.0f;
	a->f_max_hz = 1000.0f;
	a->v_min_kmh = 5.0f;
	a->y_max = 0.5f;
	a->w_max = 4.0f;
	for (k = 0U; k < ICOM_SH_N; k++) {
		a->sh_re[k] = 1.0f;
		a->sh_im[k] = 0.0f;
	}
	a->ms_e = 0.0f;
	a->ms_d_est = 0.0f;
	icom_anr_reset(a);
}

void icom_anr_reset(icom_anr_t *a)
{
	uint32_t h;

	for (h = 0U; h < ICOM_ANR_HMAX; h++) {
		a->w[h][0] = 0.0f;
		a->w[h][1] = 0.0f;
	}
	a->phase = 0.0f;
	a->ms_ref = 0.0f;
	a->t_on = 0U;
	a->t_bad = 0U;
	a->t_obs = 0U;
	a->enabled = false;
	a->fault = false;
}

void icom_anr_set_path(icom_anr_t *a, float gain, float delay_s)
{
	uint32_t k;

	for (k = 0U; k < ICOM_SH_N; k++) {
		const float ph = -TWO_PI * ((float)k * ICOM_SH_STEP_HZ) * delay_s;
		a->sh_re[k] = gain * cosf(ph);
		a->sh_im[k] = gain * sinf(ph);
	}
}

void icom_anr_rotate(icom_anr_t *a, float deg)
{
	const float r = deg * (TWO_PI / 360.0f);
	const float c = cosf(r);
	const float s = sinf(r);
	uint32_t k;

	for (k = 0U; k < ICOM_SH_N; k++) {
		const float re = a->sh_re[k];
		a->sh_re[k] = (re * c) - (a->sh_im[k] * s);
		a->sh_im[k] = (re * s) + (a->sh_im[k] * c);
	}
}

/* linear interpolation of S_hat at f (0 <= f <= f_max) */
static void shat(const icom_anr_t *a, float f, float *re, float *im)
{
	const float x = f / ICOM_SH_STEP_HZ;
	uint32_t k = (uint32_t)x;
	float t;

	if (k >= (ICOM_SH_N - 1U)) {
		k = ICOM_SH_N - 2U;
	}
	t = x - (float)k;
	*re = a->sh_re[k] + (t * (a->sh_re[k + 1U] - a->sh_re[k]));
	*im = a->sh_im[k] + (t * (a->sh_im[k + 1U] - a->sh_im[k]));
}

static void trip(icom_anr_t *a)
{
	uint32_t h;

	for (h = 0U; h < ICOM_ANR_HMAX; h++) {
		a->w[h][0] = 0.0f;
		a->w[h][1] = 0.0f;
	}
	a->enabled = false;
	a->fault = true;
}

void icom_anr_frame(icom_anr_t *a, const float err[ICOM_N], float speed_kmh, float anti[ICOM_N])
{
	const float f0 = (speed_kmh / 3.6f) / a->pitch_m;
	/* the guard reference (error without ANR) needs WARMUP samples first */
	const bool run = (!a->fault) && (a->t_obs >= WARMUP) && (speed_kmh >= a->v_min_kmh) &&
			 (f0 >= a->f_min_hz);
	float gr[ICOM_ANR_HMAX];
	float gi[ICOM_ANR_HMAX];
	float gn[ICOM_ANR_HMAX];
	bool use[ICOM_ANR_HMAX];
	uint32_t h;
	uint32_t i;

	if (run && (!a->enabled)) {
		a->enabled = true;
		a->ms_ref = a->ms_e;
		a->t_on = 0U;
		a->t_bad = 0U;
	}
	if ((!run) && a->enabled) {
		a->enabled = false;
		for (h = 0U; h < ICOM_ANR_HMAX; h++) {
			a->w[h][0] = 0.0f;
			a->w[h][1] = 0.0f;
		}
	}
	for (h = 0U; h < ICOM_ANR_HMAX; h++) {
		const float fh = ((float)h + 1.0f) * f0;
		use[h] = a->enabled && (h < a->n_harm) && (fh <= a->f_max_hz);
		gr[h] = 0.0f;
		gi[h] = 0.0f;
		gn[h] = 1.0f;
		if (use[h]) {
			shat(a, fh, &gr[h], &gi[h]);
			gn[h] = (gr[h] * gr[h]) + (gi[h] * gi[h]) + 1.0e-9f;
		}
	}
	for (i = 0U; i < ICOM_N; i++) {
		const float e = err[i];
		float y = 0.0f;

		a->ms_e += MS_ALPHA * ((e * e) - a->ms_e);
		a->phase += TWO_PI * f0 / (float)ICOM_FS_HZ;
		if (a->phase >= TWO_PI) {
			a->phase -= TWO_PI;
		}
		for (h = 0U; h < ICOM_ANR_HMAX; h++) {
			if (use[h]) {
				const float ph = ((float)h + 1.0f) * a->phase;
				const float c = cosf(ph);
				const float s = sinf(ph);
				const float k = a->mu * e / gn[h];
				a->w[h][0] -= k * ((gr[h] * c) - (gi[h] * s));
				a->w[h][1] -= k * ((gr[h] * s) + (gi[h] * c));
				y += (a->w[h][0] * c) + (a->w[h][1] * s);
				if ((fabsf(a->w[h][0]) > a->w_max) || (fabsf(a->w[h][1]) > a->w_max)) {
					a->t_bad = GUARD_TRIP;
				}
			}
		}
		if (y > a->y_max) {
			y = a->y_max;
		} else if (y < -a->y_max) {
			y = -a->y_max;
		} else {
			/* within the clamp */
		}
		anti[i] = a->enabled ? y : 0.0f;
	}
	if (a->enabled) {
		a->t_on += ICOM_N;
		if ((a->t_on >= GUARD_SETTLE) && (a->ms_e > (GUARD_RATIO * a->ms_ref))) {
			a->t_bad += ICOM_N;
		} else if (a->t_bad < GUARD_TRIP) {
			a->t_bad = 0U;
		} else {
			/* weight guard already tripped */
		}
		if (a->t_bad >= GUARD_TRIP) {
			trip(a);
		}
	} else {
		a->ms_d_est = a->ms_e;
		if (a->t_obs < WARMUP) {
			a->t_obs += ICOM_N;
		}
	}
}

uint16_t icom_anr_atten_x10(const icom_anr_t *a)
{
	uint16_t r = 0U;

	if (a->enabled && (a->ms_ref > 0.0f)) {
		/* ms_ref <= 1 (full scale) and the denominator >= 1e-30, so db < 301 */
		const float db = 10.0f * log10f(a->ms_ref / (a->ms_e + 1.0e-30f));
		if (db > 0.0f) {
			r = (uint16_t)(db * 10.0f);
		}
	}
	return r;
}
