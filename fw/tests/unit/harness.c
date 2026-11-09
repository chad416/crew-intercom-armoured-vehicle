/*
 * Test runner and plant model.
 * SPDX-License-Identifier: MIT
 */
#include "harness.h"

int g_checks;
int g_failures;

double frand(unsigned *s)
{
	*s = *s * 1103515245u + 12345u;
	return ((double)((*s >> 8) & 0xFFFFFFu) / 16777216.0) * 2.0 - 1.0;
}

void plant_init(plant_t *p, double pitch, int n_harm, const double *amp, int delay, double gain)
{
	memset(p, 0, sizeof(*p));
	p->fs = (double)ICOM_FS_HZ;
	p->pitch = pitch;
	p->n_harm = n_harm;
	for (int h = 0; h < n_harm; h++) {
		p->amp[h] = amp[h];
		p->ph0[h] = 0.7 * (h + 1);
	}
	p->delay = delay;
	p->gain = gain;
	p->rng = 12345u;
}

void plant_frame(plant_t *p, double speed_kmh, const float anti[ICOM_N], float err[ICOM_N], double *d_ms)
{
	double f0 = speed_kmh / 3.6 / p->pitch;
	double acc = 0.0;
	for (unsigned i = 0; i < ICOM_N; i++) {
		p->phase += 2.0 * M_PI * f0 / p->fs;
		double d = 0.0;
		for (int h = 0; h < p->n_harm; h++) {
			d += p->amp[h] * cos((h + 1) * p->phase + p->ph0[h]);
		}
		d += p->noise * frand(&p->rng);
		acc += d * d;
		/* anti-noise enters the delay line now and appears delay samples later */
		p->line[p->idx] = anti ? anti[i] : 0.0;
		int out = (p->idx - p->delay + PLANT_DMAX) % PLANT_DMAX;
		err[i] = (float)(d + p->gain * p->line[out]);
		p->idx = (p->idx + 1) % PLANT_DMAX;
	}
	if (d_ms) {
		*d_ms = acc / ICOM_N;
	}
}

int main(void)
{
	test_frame();
	test_link();
	test_modes();
	test_mix();
	test_anr();
	test_box();
	test_ccu();
	test_system();
	printf("%d checks, %d failures\n", g_checks, g_failures);
	return g_failures ? 1 : 0;
}
