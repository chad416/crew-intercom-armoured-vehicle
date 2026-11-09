"""Active noise reduction (ANR) models for the crew headset.

Two mechanisms, because they have opposite latency behaviour:

1. Speed-referenced harmonic FxLMS ("order cancellation").  Tracked-vehicle
   interior noise below ~1 kHz is dominated by the track-laying frequency
   f0 = v / p (v vehicle speed, p track pitch) and its harmonics.  A synthetic
   reference (sin/cos of h * phase(f0)) is built from the speed signal, so the
   controller does not need a reference microphone and is not limited by
   causality: a pure delay in the secondary path only rotates the phase that
   the filtered-x estimate must match.  Stability needs the phase error of the
   secondary-path estimate to stay within +/-90 deg (Morgan 1980; Kuo & Morgan
   1996, ch. 3) and a step size below the delayed-LMS bound.

2. Broadband feedback ANR.  For a random (non-periodic) noise, a feedback
   controller with total loop delay D samples can at best remove the part of
   the noise that is predictable D samples ahead.  ``predictable_attenuation_db``
   computes that bound from the noise spectrum (Wiener/Kolmogorov one-sided
   prediction error), which shows how quickly broadband ANR collapses when the
   loop goes through a block-based audio pipeline.

The anti-noise must not cancel the intercom audio that the same earphone
plays: the error signal is "audio-compensated" (e_c = e - S_hat * audio)
before it drives the adaptation.
"""

from __future__ import annotations

import math

import numpy as np
from scipy import signal

# ------------------------------------------------------------------ paths


class SecondaryPath:
    """Earphone -> error-microphone path: pure delay + 2nd-order band-pass.

    delay_s models converter group delay plus processing/block delay.
    """

    def __init__(self, fs, delay_s, gain=1.0, f_lo=30.0, f_hi=4000.0):
        self.fs = fs
        self.d = int(round(delay_s * fs))
        self.gain = gain
        self.b, self.a = signal.butter(1, [f_lo, min(f_hi, 0.45 * fs)], btype="bandpass", fs=fs)

    def response(self, f):
        """Complex frequency response at frequency f (Hz)."""
        _, h = signal.freqz(self.b, self.a, worN=[f], fs=self.fs)
        return complex(self.gain * h[0] * np.exp(-2j * np.pi * f * self.d / self.fs))

    def apply(self, y):
        z = signal.lfilter(self.b, self.a, y) * self.gain
        return np.concatenate([np.zeros(self.d), z[: len(z) - self.d]]) if self.d else z


# ------------------------------------------------------------------ noise


def track_noise(fs, seconds, speed_kmh, pitch_m, harm_db, broadband=None, rng=None, phase_noise=0.0):
    """Tonal track-laying noise (Pa) for a speed profile.

    speed_kmh: scalar or array (len = n samples); harm_db: SPL (dB re 20 uPa) of
    harmonics 1..H.  Returns (pressure, f0 array, phase array of f0).
    """
    n = int(fs * seconds)
    v = np.broadcast_to(np.asarray(speed_kmh, dtype=float), (n,)) / 3.6
    f0 = v / pitch_m
    ph = 2 * np.pi * np.cumsum(f0) / fs
    if phase_noise and rng is not None:
        ph = ph + np.cumsum(rng.standard_normal(n)) * phase_noise
    p = np.zeros(n)
    rng = rng or np.random.default_rng(0)
    for h, lv in enumerate(harm_db, start=1):
        amp = math.sqrt(2.0) * 20e-6 * 10 ** (lv / 20.0)
        p += amp * np.cos(h * ph + rng.uniform(0, 2 * np.pi))
    if broadband is not None:
        p = p + broadband[:n]
    return p, f0, ph


# ------------------------------------------------------------------ FxLMS


def harmonic_fxlms(
    d,
    f0_meas,
    fs,
    path,
    n_harm,
    mu=0.01,
    s_hat_phase_err_deg=0.0,
    s_hat_gain_err=1.0,
    audio=None,
    audio_comp=True,
    leak=0.0,
    hist_every_s=0.01,
):
    """Run the speed-referenced harmonic FxLMS controller, sample by sample.

    d: disturbance at the error microphone (Pa); f0_meas: the controller's
    estimate of f0 per sample (from the speed signal, may differ from truth);
    path: SecondaryPath (truth).  The controller's estimate S_hat is the true
    response rotated by s_hat_phase_err_deg and scaled by s_hat_gain_err; in the
    firmware it is a table of complex gains measured at commissioning.
    audio: intercom audio (Pa at the error mic, before the path) that the
    earphone plays together with the anti-noise.  With audio_comp the error is
    compensated by S_hat * audio before adaptation.
    mu is normalised: the update is mu * e * x_f / |S_hat|^2.
    Returns dict(e=error signal, y=controller output, w=weight history,
    diverged flag).
    """
    n = len(d)
    w = [[0.0, 0.0] for _ in range(n_harm)]
    y = np.zeros(n)
    e = np.zeros(n)
    rot = math.radians(s_hat_phase_err_deg)
    grid = np.linspace(1.0, 0.45 * fs, 4096)
    _, hh = signal.freqz(path.b, path.a, worN=grid, fs=fs)
    sh = path.gain * hh * np.exp(-2j * np.pi * grid * path.d / fs) * s_hat_gain_err * np.exp(1j * rot)
    sh_re, sh_im = sh.real.tolist(), sh.imag.tolist()
    step = grid[1] - grid[0]
    b0, b1, b2 = (float(x) for x in path.b)
    _, a1, a2 = (float(x) for x in path.a)
    g = path.gain
    D = path.d
    ring = [0.0] * (D + 1)
    ring_hat = [0.0] * (D + 1)
    x1 = x2 = y1 = y2 = 0.0
    u1 = u2 = v1 = v2 = 0.0
    aud = [0.0] * n if audio is None else list(map(float, audio))
    dd = list(map(float, d))
    f0l = list(map(float, f0_meas))
    hist = []
    every = max(int(hist_every_s * fs), 1)
    diverged = False
    phase = 0.0
    two_pi_fs = 2 * math.pi / fs
    kmax = len(grid) - 1
    ridx = 0
    for i in range(n):
        phase += two_pi_fs * f0l[i]
        out = 0.0
        cs = []
        for h in range(n_harm):
            c, s_ = math.cos((h + 1) * phase), math.sin((h + 1) * phase)
            out += w[h][0] * c + w[h][1] * s_
            cs.append((c, s_))
        y[i] = out
        drv = out + aud[i]
        z = b0 * drv + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
        x2, x1, y2, y1 = x1, drv, y1, z
        ring[ridx] = z * g
        ei = dd[i] + ring[(ridx - D) % (D + 1)]
        e[i] = ei
        if audio is not None and audio_comp:
            za = b0 * aud[i] + b1 * u1 + b2 * u2 - a1 * v1 - a2 * v2
            u2, u1, v2, v1 = u1, aud[i], v1, za
            ring_hat[ridx] = za * g * s_hat_gain_err
            ei -= ring_hat[(ridx - D) % (D + 1)]
        ridx = (ridx + 1) % (D + 1)
        for h in range(n_harm):
            k = min(int((h + 1) * f0l[i] / step), kmax)
            gr, gi = sh_re[k], sh_im[k]
            c, s_ = cs[h]
            nrm = gr * gr + gi * gi + 1e-12
            wh = w[h]
            wh[0] = (1.0 - leak) * wh[0] - mu * ei * (gr * c - gi * s_) / nrm
            wh[1] = (1.0 - leak) * wh[1] - mu * ei * (gr * s_ + gi * c) / nrm
        if i % every == 0:
            hist.append([v for wh in w for v in wh])
            if not all(math.isfinite(v) and abs(v) < 1e3 for v in hist[-1]):
                diverged = True
                e[i:] = np.nan
                break
    return {"e": e, "y": y, "w": np.array(hist), "diverged": diverged}


# ------------------------------------------------------------------ broadband bound


def predictable_attenuation_db(psd_f, psd, fs, delay_s, order=48):
    """Upper bound of broadband feedback-ANR attenuation for a loop delay.

    Fits an AR(order) model to the noise (via its autocorrelation from the PSD)
    and returns 10*log10(var / var_of_D-step_prediction_error).  A loop with
    delay D cannot cancel what is not predictable D samples ahead.
    """
    nfft = 1 << 16
    f = np.fft.rfftfreq(nfft, 1.0 / fs)
    p = np.interp(f, psd_f, psd, left=psd[0], right=0.0)
    r = np.fft.irfft(p, n=nfft)[: order + 1]
    a = _levinson(r, order)
    # impulse response of the innovations filter 1/A(z)
    D = max(int(round(delay_s * fs)), 1)
    imp = np.zeros(D)
    imp[0] = 1.0
    h = signal.lfilter([1.0], a, imp)
    sig2 = r[0]
    e_var = r[0] * 0 + _innov_var(r, a) * np.sum(h[:D] ** 2)
    return 10.0 * np.log10(sig2 / max(e_var, 1e-30 * sig2))


def _levinson(r, order):
    a = np.zeros(order + 1)
    a[0] = 1.0
    err = r[0]
    for k in range(1, order + 1):
        acc = r[k] + np.dot(a[1:k], r[k - 1 : 0 : -1])
        ref = -acc / err
        a_new = a.copy()
        a_new[1:k] = a[1:k] + ref * a[k - 1 : 0 : -1]
        a_new[k] = ref
        a = a_new
        err *= 1.0 - ref * ref
    return a


def _innov_var(r, a):
    return float(np.dot(a, r[: len(a)]))
