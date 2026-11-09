/*
 * Speed-referenced harmonic ANR (CB-40..CB-47), closed loop with the plant model.
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

#define DELAY 32 /* plant delay: 2 ms; plus one 1 ms frame in the pipeline */

static const double k_amp[3] = {0.10, 0.07, 0.05};

/* Run the loop for seconds; return attenuation (dB) over the last 0.5 s. */
static double run(icom_anr_t *a, plant_t *p, double speed, double seconds)
{
	float err[ICOM_N], anti[ICOM_N];
	double d_acc = 0.0, e_acc = 0.0, d_ms;
	unsigned frames = (unsigned)(seconds * 1000.0);
	memset(anti, 0, sizeof(anti));
	for (unsigned f = 0; f < frames; f++) {
		plant_frame(p, speed, anti, err, &d_ms);
		icom_anr_frame(a, err, (float)speed, anti);
		if (f >= frames - 500) {
			d_acc += d_ms;
			for (unsigned i = 0; i < ICOM_N; i++) e_acc += (double)err[i] * err[i] / ICOM_N;
		}
	}
	return 10.0 * log10(d_acc / (e_acc + 1e-30));
}

static void setup(icom_anr_t *a, plant_t *p, unsigned n_harm)
{
	icom_anr_init(a, 0.15f, n_harm, 0.002f);
	icom_anr_set_path(a, 1.0f, (float)(DELAY + ICOM_N) / (float)ICOM_FS_HZ);
	plant_init(p, 0.15, 3, k_amp, DELAY, 1.0);
}

void test_anr(void)
{
	icom_anr_t a;
	plant_t p;
	float err[ICOM_N], anti[ICOM_N];

	/* CB-40/41: converges on 3 harmonics at 50 km/h with a 3 ms path delay */
	setup(&a, &p, 3);
	double att = run(&a, &p, 50.0, 3.0);
	printf("  ANR 50 km/h, exact S_hat: %.1f dB\n", att);
	CHECK(att > 40.0);
	CHECK(a.enabled);
	CHECK(!a.fault);
	CHECK(icom_anr_atten_x10(&a) > 300U);

	/* with broadband noise in the error signal: tones still removed */
	setup(&a, &p, 3);
	p.noise = 0.02;
	att = run(&a, &p, 50.0, 3.0);
	printf("  ANR with broadband floor: %.1f dB total\n", att);
	CHECK(att > 10.0);

	/* CB-42: 60 deg S_hat phase error still converges */
	setup(&a, &p, 3);
	icom_anr_rotate(&a, 60.0f);
	att = run(&a, &p, 50.0, 4.0);
	printf("  ANR, 60 deg S_hat error: %.1f dB\n", att);
	CHECK(att > 30.0);
	CHECK(!a.fault);

	/* rotate there and back is the identity */
	icom_anr_rotate(&a, 90.0f);
	icom_anr_rotate(&a, -150.0f);
	CHECK_NEAR(a.sh_re[0], 1.0, 1e-5);
	CHECK_NEAR(a.sh_im[0], 0.0, 1e-5);

	/* CB-46: 120 deg error diverges; the guard trips and stays off */
	setup(&a, &p, 3);
	icom_anr_rotate(&a, 120.0f);
	run(&a, &p, 50.0, 4.0);
	CHECK(a.fault);
	CHECK(!a.enabled);
	CHECK_EQ(icom_anr_atten_x10(&a), 0);
	plant_frame(&p, 50.0, NULL, err, NULL);
	icom_anr_frame(&a, err, 50.0f, anti);
	CHECK_EQ(anti[0], 0.0f); /* no output while faulted */
	icom_anr_reset(&a);
	CHECK(!a.fault);

	/* CB-44: below v_min (or f0 < f_min) the ANR is off */
	setup(&a, &p, 3);
	run(&a, &p, 3.0, 0.2);
	CHECK(!a.enabled);
	CHECK(a.ms_d_est > 0.0f);
	icom_anr_init(&a, 1.0f, 3U, 0.002f); /* 50 km/h with 1 m pitch: f0 = 13.9 Hz */
	run(&a, &p, 50.0, 0.5);
	CHECK(!a.enabled);

	/* speed drops while running: off, weights cleared */
	setup(&a, &p, 3);
	run(&a, &p, 50.0, 0.5);
	CHECK(a.enabled);
	run(&a, &p, 2.0, 0.01);
	CHECK(!a.enabled);
	CHECK_EQ(a.w[0][0], 0.0f);

	/* CB-45: harmonics above f_max are not used; n_harm is clamped */
	icom_anr_init(&a, 0.15f, 9U, 0.002f);
	CHECK_EQ(a.n_harm, ICOM_ANR_HMAX);
	icom_anr_set_path(&a, 1.0f, (float)(DELAY + ICOM_N) / (float)ICOM_FS_HZ);
	plant_init(&p, 0.15, 3, k_amp, DELAY, 1.0);
	run(&a, &p, 150.0, 1.0); /* f0 = 278 Hz: the 4th harmonic is 1111 Hz */
	CHECK_EQ(a.w[3][0], 0.0f);
	CHECK(a.w[0][0] != 0.0f);

	/* CB-43: output clamp */
	setup(&a, &p, 3);
	a.y_max = 0.01f;
	float lo = 0.0f, hi = 0.0f;
	memset(anti, 0, sizeof(anti));
	for (int f = 0; f < 300; f++) {
		plant_frame(&p, 50.0, anti, err, NULL);
		icom_anr_frame(&a, err, 50.0f, anti);
		for (unsigned i = 0; i < ICOM_N; i++) {
			if (anti[i] > hi) hi = anti[i];
			if (anti[i] < lo) lo = anti[i];
		}
	}
	CHECK_NEAR(hi, 0.01, 1e-6);
	CHECK_NEAR(lo, -0.01, 1e-6);

	/* CB-46: weight-magnitude guard */
	setup(&a, &p, 3);
	a.w_max = 0.01f;
	run(&a, &p, 50.0, 0.5);
	CHECK(a.fault);

	/* silent warm-up: enabled with a zero guard reference, telemetry 0 */
	{
		static const double zero[3] = {0.0, 0.0, 0.0};
		icom_anr_init(&a, 0.15f, 3U, 0.002f);
		plant_init(&p, 0.15, 3, zero, DELAY, 1.0);
		run(&a, &p, 50.0, 0.3);
		CHECK(a.enabled);
		CHECK_EQ(icom_anr_atten_x10(&a), 0);
	}

	/* S_hat interpolation at the top of the table */
	setup(&a, &p, 3);
	a.f_max_hz = 2000.0f;
	plant_init(&p, 0.15, 1, k_amp, DELAY, 1.0);
	run(&a, &p, 600.0, 0.5); /* f0 = 1111 Hz: beyond the table, clamped */
	CHECK(a.enabled);
}
