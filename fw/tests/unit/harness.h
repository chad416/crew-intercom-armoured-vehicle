/*
 * Minimal test harness and an acoustic plant for the ANR tests.
 * SPDX-License-Identifier: MIT
 */
#ifndef ICOM_TEST_HARNESS_H
#define ICOM_TEST_HARNESS_H

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "icom/icom.h"

extern int g_checks;
extern int g_failures;

#define CHECK(cond)                                                                    \
	do {                                                                           \
		g_checks++;                                                            \
		if (!(cond)) {                                                         \
			g_failures++;                                                  \
			printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
		}                                                                      \
	} while (0)

#define CHECK_EQ(a, b)                                                                 \
	do {                                                                           \
		long long _a = (long long)(a);                                         \
		long long _b = (long long)(b);                                         \
		g_checks++;                                                            \
		if (_a != _b) {                                                        \
			g_failures++;                                                  \
			printf("FAIL %s:%d: %s == %s (%lld != %lld)\n", __FILE__,      \
			       __LINE__, #a, #b, _a, _b);                              \
		}                                                                      \
	} while (0)

#define CHECK_NEAR(a, b, tol)                                                          \
	do {                                                                           \
		double _a = (double)(a);                                               \
		double _b = (double)(b);                                               \
		g_checks++;                                                            \
		if (fabs(_a - _b) > (double)(tol)) {                                   \
			g_failures++;                                                  \
			printf("FAIL %s:%d: %s ~ %s (%g vs %g, tol %g)\n", __FILE__,    \
			       __LINE__, #a, #b, _a, _b, (double)(tol));               \
		}                                                                      \
	} while (0)

/* ---- Acoustic plant: tonal disturbance + secondary path (delay, gain, phase) */

#define PLANT_DMAX 512

typedef struct {
	double fs;
	double pitch;
	int n_harm;
	double amp[ICOM_ANR_HMAX];
	double ph0[ICOM_ANR_HMAX];
	double phase;
	int delay;          /* samples, includes the frame pipeline */
	double gain;
	double line[PLANT_DMAX];
	int idx;
	double noise;       /* broadband noise rms (full scale) */
	unsigned rng;
} plant_t;

void plant_init(plant_t *p, double pitch, int n_harm, const double *amp, int delay, double gain);
/* produce one frame of error-mic samples given last frame's anti-noise */
void plant_frame(plant_t *p, double speed_kmh, const float anti[ICOM_N], float err[ICOM_N], double *d_ms);
double frand(unsigned *s);

/* ---- suites */
void test_frame(void);
void test_link(void);
void test_modes(void);
void test_mix(void);
void test_anr(void);
void test_box(void);
void test_ccu(void);
void test_system(void);

#endif
