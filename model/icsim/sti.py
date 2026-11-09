"""Speech Transmission Index (STI) after IEC 60268-16 ed. 4 (2011) / ed. 5 (2020).

Two methods:

* ``sti_indirect`` - computes the modulation transfer function (MTF) from
  octave-band speech and noise levels at the listener (plus an optional
  linear-system MTF), then applies the auditory-masking and absolute
  reception-threshold corrections and the male weighting factors.
* ``stipa_signal`` / ``sti_direct`` - the STIPA direct method: a modulated
  noise test signal is passed through the system under test (here: a
  simulation, including time-varying adaptive filters) and the received
  modulation depths are measured.  The direct method is what makes it legal
  to put a non-LTI block (an adaptive filter) in the chain; the indirect
  method assumes an LTI channel with stationary noise.

Constants are those of IEC 60268-16 ed. 4 Annex A (male talker); the
sources and the checks against them are listed in docs/sources.md.
"""

from __future__ import annotations

import numpy as np
from scipy import signal

BANDS = np.array([125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0])
MOD_FREQS = np.array([0.63, 0.8, 1.0, 1.25, 1.6, 2.0, 2.5, 3.15, 4.0, 5.0, 6.3, 8.0, 10.0, 12.5])
ALPHA = np.array([0.085, 0.127, 0.230, 0.233, 0.309, 0.224, 0.173])
BETA = np.array([0.085, 0.078, 0.065, 0.011, 0.047, 0.095])
# absolute speech reception threshold, dB SPL per octave band
ART = np.array([46.0, 27.0, 12.0, 6.5, 7.5, 8.0, 12.0])
# STIPA modulation frequencies per band (two per band)
STIPA_FM = np.array(
    [[1.60, 8.00], [1.00, 5.00], [0.63, 3.15], [2.00, 10.0], [1.25, 6.25], [0.80, 4.00], [2.50, 12.5]]
)
STIPA_M = 0.55


def db2i(level_db):
    return 10.0 ** (np.asarray(level_db, dtype=float) / 10.0)


def i2db(intensity):
    return 10.0 * np.log10(np.maximum(np.asarray(intensity, dtype=float), 1e-30))


def masking_db(level_db):
    """Auditory masking level (dB relative to the band below), IEC 60268-16 ed. 4 Table A.?.

    The four segments join continuously at 63, 67 and 100 dB.
    """
    lv = np.asarray(level_db, dtype=float)
    return np.where(
        lv < 63.0,
        0.5 * lv - 65.0,
        np.where(lv < 67.0, 1.8 * lv - 146.9, np.where(lv < 100.0, 0.5 * lv - 59.8, -10.0)),
    )


def correct_mtf(m, band_level_db):
    """Apply masking from band k-1 and the reception threshold to an MTF matrix.

    m: (7, n_f) modulation transfer values; band_level_db: (7,) total (speech +
    noise) octave-band level at the listener in dB SPL.
    """
    i_k = db2i(band_level_db)
    i_am = np.zeros(7)
    i_am[1:] = i_k[:-1] * db2i(masking_db(band_level_db[:-1]))
    i_rt = db2i(ART)
    return m * (i_k / (i_k + i_am + i_rt))[:, None]


def mti_from_m(m):
    m = np.clip(m, 1e-9, 1.0 - 1e-9)
    snr = np.clip(10.0 * np.log10(m / (1.0 - m)), -15.0, 15.0)
    ti = (snr + 15.0) / 30.0
    return ti.mean(axis=1)


def sti_from_mti(mti):
    mti = np.asarray(mti, dtype=float)
    return float(np.sum(ALPHA * mti) - np.sum(BETA * np.sqrt(mti[:-1] * mti[1:])))


def sti_indirect(speech_db, noise_db, system_mtf=None, masking=True, freqs=MOD_FREQS):
    """STI from octave-band speech and noise levels at the listener (dB SPL).

    system_mtf: optional (7, n_f) MTF of the linear system (reverberation, echo);
    noise_db may contain -inf / very low values for "no noise".
    Returns (sti, mti[7]).
    """
    s = db2i(speech_db)
    n = db2i(noise_db)
    m = np.ones((7, len(freqs))) if system_mtf is None else np.array(system_mtf, dtype=float)
    m = m * (s / (s + n))[:, None]
    if masking:
        m = correct_mtf(m, i2db(s + n))
    mti = mti_from_m(m)
    return sti_from_mti(mti), mti


def rating(sti):
    """Intelligibility rating band of ISO 9921:2003 (Table 2 of IEC 60268-16 ed. 4 Annex)."""
    for lim, name in ((0.75, "excellent"), (0.60, "good"), (0.45, "fair"), (0.30, "poor")):
        if sti > lim:
            return name
    return "bad"


# ----------------------------------------------------------------- direct method


def octave_sos(fc, fs, order=3):
    lo, hi = fc / np.sqrt(2.0), min(fc * np.sqrt(2.0), 0.45 * fs)
    return signal.butter(order, [lo, hi], btype="bandpass", fs=fs, output="sos")


def stipa_signal(band_levels_db, fs, seconds, rng):
    """STIPA test signal (Pa) whose octave bands have the given levels (dB SPL).

    Each band: a half-octave noise carrier, intensity-modulated at the band's two
    STIPA frequencies with m = 0.55 each.
    """
    n = int(fs * seconds)
    t = np.arange(n) / fs
    out = np.zeros(n)
    for k, fc in enumerate(BANDS):
        lo, hi = fc / 2**0.25, min(fc * 2**0.25, 0.45 * fs)
        sos = signal.butter(4, [lo, hi], btype="bandpass", fs=fs, output="sos")
        car = signal.sosfilt(sos, rng.standard_normal(n))
        f1, f2 = STIPA_FM[k]
        env = 1.0 + STIPA_M * np.cos(2 * np.pi * f1 * t) + STIPA_M * np.cos(2 * np.pi * f2 * t)
        x = car * np.sqrt(np.maximum(env, 0.0))
        p_rms = 20e-6 * 10 ** (band_levels_db[k] / 20.0)
        out += x * (p_rms / np.sqrt(np.mean(x * x)))
    return out


def octave_split(x, fs):
    """Split a signal into the 7 octave bands with ideal (FFT brick-wall) filters.

    A real STIPA meter uses IEC 61260 class 1 filters; brick-wall filters remove
    the leakage between neighbouring bands that otherwise biases the clean-channel
    STI low (0.97 instead of 1.00 with 6th-order Butterworth bands).
    """
    spec = np.fft.rfft(x)
    f = np.fft.rfftfreq(len(x), 1.0 / fs)
    out = np.zeros((7, len(x)))
    for k, fc in enumerate(BANDS):
        sel = (f >= fc / np.sqrt(2.0)) & (f < fc * np.sqrt(2.0))
        out[k] = np.fft.irfft(np.where(sel, spec, 0.0), n=len(x))
    return out


def band_levels(x, fs):
    """Octave-band SPL (dB re 20 uPa) of a pressure signal."""
    return i2db(np.mean(octave_split(x, fs) ** 2, axis=1) / (20e-6) ** 2)


def sti_direct(received, fs, masking=True, skip_s=1.0):
    """Measure STI (STIPA) from the received pressure signal.

    The first skip_s seconds are discarded (filter and adaptive-filter settling);
    the analysis length is cut to a whole number of periods of 0.63 Hz.
    """
    x = received[int(skip_s * fs) :]
    per = int(round(fs / 0.63 * np.floor(len(x) / (fs / 0.63))))
    x = x[: max(per, int(fs * 8))]
    t = np.arange(len(x)) / fs
    m = np.zeros((7, 2))
    lev = np.zeros(7)
    lp = signal.butter(4, 100.0, fs=fs, output="sos")
    ys = octave_split(x, fs)
    for k in range(7):
        y = ys[k]
        inten = signal.sosfiltfilt(lp, y * y)
        lev[k] = i2db(np.mean(y * y) / (20e-6) ** 2)
        for j, fm in enumerate(STIPA_FM[k]):
            z = np.sum(inten * np.exp(-2j * np.pi * fm * t))
            m[k, j] = 2.0 * np.abs(z) / np.sum(inten) / STIPA_M
    m = np.minimum(m, 1.0)
    if masking:
        m = correct_mtf(m, lev)
    mti = mti_from_m(m)
    return sti_from_mti(mti), mti, lev
