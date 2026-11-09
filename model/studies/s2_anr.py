"""Study S2 - adaptive ANR: convergence, stability and the latency trade.

A. learning curves of the speed-referenced harmonic FxLMS for several step sizes
B. stability: final attenuation vs phase error of S_hat (cliff at 90 deg)
C. latency: tonal FxLMS (delay-tolerant) vs the prediction bound of broadband
   feedback ANR (delay-limited)
D. tracking vs misadjustment: attenuation vs step size for tonal line widths
   0 / 1 / 3 Hz and speed-signal error

Tones: track pitch 0.149 m, 40 km/h -> f0 = 74.6 Hz [R1]; three harmonics.
Outputs: docs/img/s2_*.png, results/s2_anr.json
"""

from __future__ import annotations

import numpy as np

from common import plt, save, dump  # noqa: F401
from icsim import anr, chain, data

FS = 4000
PITCH = data.TRACK_PITCH_M
HARM_DB = [100.0, 96.0, 92.0]  # tone levels at the ear under the helmet (assumption, ratio from [R2])


def pa_db(x):
    return 10 * np.log10(np.nanmean(x * x) / (20e-6) ** 2)


def linewidth_sigma(hz):
    """Phase random-walk step (rad/sample) for a Lorentzian line of width hz."""
    return float(np.sqrt(2 * np.pi * hz / FS)) if hz > 0 else 0.0


def scenario(seconds, speed=40.0, lw_hz=0.0, speed_err=0.0, bb_db=None, seed=1, jitter_kmh=0.2, ref="sprocket"):
    """Tonal track noise for a speed profile with random speed jitter (FM of the tones).

    ref="sprocket": the controller gets the track-link phase from a sprocket pulse pick-up,
    i.e. the true instantaneous f0 (phase-coherent with the tones).
    ref="bus": the controller gets the speed value from the vehicle bus at 50 Hz (zero-order
    hold), so the speed jitter between samples is invisible to it.
    lw_hz: extra random phase wander that no sensor can see (Lorentzian line width).
    """
    from scipy.signal import butter, lfilter

    rng = np.random.default_rng(seed)
    n = int(FS * seconds)
    t = np.arange(n) / FS
    b2, a2 = butter(2, 2.0, fs=FS)  # speed jitter band-limited to ~2 Hz
    j = lfilter(b2, a2, rng.standard_normal(n))
    j = j / (np.std(j) + 1e-12) * jitter_kmh if jitter_kmh > 0 else 0.0 * j
    v = speed + 2.0 * np.sin(2 * np.pi * t / 5.0) + j  # slow manoeuvre + jitter
    bb = None
    if bb_db is not None:
        b, a = butter(2, [40, 600], btype="bandpass", fs=FS)
        x = lfilter(b, a, rng.standard_normal(n))
        bb = x * (20e-6 * 10 ** (bb_db / 20) / np.std(x))
    d, f0, _ = anr.track_noise(FS, seconds, v, PITCH, HARM_DB, broadband=bb, rng=rng,
                               phase_noise=linewidth_sigma(lw_hz))
    if ref == "sprocket":
        f0_meas = f0 * (1.0 + speed_err)
    else:
        idx = (np.arange(n) // (FS // 50)) * (FS // 50)
        f0_meas = f0[idx] * (1.0 + speed_err)
    return d, f0_meas, bb


def tonal_residual_db(d, e, bb, tail):
    """Attenuation of the tonal part: (tone power) / (error power - broadband power)."""
    sl = slice(len(d) - tail, len(d))
    p_d = np.mean(d[sl] ** 2) - (np.mean(bb[sl] ** 2) if bb is not None else 0.0)
    p_e = np.nanmean(e[sl] ** 2) - (np.mean(bb[sl] ** 2) if bb is not None else 0.0)
    return 10 * np.log10(p_d / max(p_e, 1e-12 * p_d))


def study_a():
    d, f0m, _ = scenario(6.0)
    path = anr.SecondaryPath(FS, 0.003)
    out = {}
    fig, ax = plt.subplots(figsize=(4.6, 3.2))
    win = FS // 20
    for mu in (0.001, 0.003, 0.01, 0.03):
        r = anr.harmonic_fxlms(d, f0m, FS, path, 3, mu=mu)
        e2 = np.convolve(r["e"] ** 2, np.ones(win) / win, mode="valid")
        d2 = np.convolve(d ** 2, np.ones(win) / win, mode="valid")
        att = 10 * np.log10(d2 / e2)
        tt = (np.arange(len(att)) + win / 2) / FS
        ax.plot(tt, att, label="mu = %g" % mu)
        t90 = tt[np.argmax(att >= 0.9 * np.nanmax(att[-FS:]))]
        out["mu=%g" % mu] = {"final_db": float(np.nanmean(att[-FS:])), "t_90pct_s": float(t90)}
    ax.set_xlabel("time (s)")
    ax.set_ylabel("attenuation at the error mic (dB)")
    ax.set_title("FxLMS learning: 3 harmonics, sprocket reference, 3 ms path", fontsize=9)
    ax.legend(fontsize=7)
    return save(fig, "s2_learning.png"), out


def study_b():
    errs = np.arange(0, 151, 10)
    out = {}
    fig, ax = plt.subplots(figsize=(4.6, 3.2))
    d, f0m, _ = scenario(5.0)
    for delay in (0.001, 0.003, 0.010):
        path = anr.SecondaryPath(FS, delay)
        ys = []
        for pe in errs:
            r = anr.harmonic_fxlms(d, f0m, FS, path, 3, mu=0.01, s_hat_phase_err_deg=float(pe))
            if r["diverged"]:
                ys.append(np.nan)
            else:
                tail = slice(len(d) - FS, len(d))
                ys.append(10 * np.log10(np.mean(d[tail] ** 2) / np.mean(r["e"][tail] ** 2)))
        ax.plot(errs, np.minimum(ys, 60), marker=".", label="path delay %g ms" % (delay * 1e3))
        out["delay_ms=%g" % (delay * 1e3)] = ys
    ax.axvline(90, color="k", ls="--", lw=0.8)
    ax.set_xlabel("phase error of S_hat (deg)")
    ax.set_ylabel("attenuation after 4-5 s (dB, capped 60)")
    ax.set_title("Stability: diverges (gap) beyond 90 deg", fontsize=9)
    ax.legend(fontsize=7)
    return save(fig, "s2_stability.png"), {"phase_err_deg": errs, **out}


def study_c():
    # broadband part of the tracked ear noise (helmet only), as a PSD over 20-2000 Hz
    c = chain.Config("x")
    t = chain.tonal_share("tracked")
    nb = chain.ear_noise(c) + 10 * np.log10(np.maximum(1 - t, 1e-3))
    f_oct = data.OCT
    fs = 16000
    f = np.linspace(1, fs / 2, 4000)
    lev = np.interp(np.log2(f), np.log2(f_oct), nb, left=nb[0], right=nb[-1])
    psd = 10 ** (lev / 10) / (f_oct[np.clip(np.searchsorted(f_oct, f), 0, 7)] * 0.707)
    psd[f < 20] = psd[f >= 20][0] * 1e-4
    delays = np.array([0.05, 0.1, 0.2, 0.5, 1.0, 2.0, 4.0, 8.0]) * 1e-3
    bb = [anr.predictable_attenuation_db(f, psd, fs, dl) for dl in delays]
    # tonal: FxLMS with the exact S_hat at the same delays
    d, f0m, _ = scenario(5.0)
    ton = []
    for dl in delays:
        path = anr.SecondaryPath(FS, max(dl, 1.0 / FS))
        r = anr.harmonic_fxlms(d, f0m, FS, path, 3, mu=0.01)
        tail = slice(len(d) - FS, len(d))
        ton.append(10 * np.log10(np.mean(d[tail] ** 2) / np.mean(r["e"][tail] ** 2)))
    fig, ax = plt.subplots(figsize=(4.6, 3.2))
    ax.semilogx(delays * 1e3, ton, marker="o", label="tonal FxLMS (sprocket reference)")
    ax.semilogx(delays * 1e3, bb, marker="s", label="broadband feedback ANR: prediction bound")
    ax.axvspan(2.4, 10, color="0.9", lw=0)
    ax.text(2.6, max(ton) * 0.92, "1 ms frame\npipeline", fontsize=7)
    ax.set_xlabel("total loop delay (ms)")
    ax.set_ylabel("best attenuation (dB)")
    ax.set_title("Latency decides where each ANR can run", fontsize=9)
    ax.legend(fontsize=7)
    return save(fig, "s2_latency.png"), {"delay_ms": delays * 1e3, "tonal_db": ton, "broadband_bound_db": bb}


def study_d():
    mus = [0.001, 0.003, 0.01, 0.03, 0.1]
    out = {}
    fig, ax = plt.subplots(figsize=(4.6, 3.2))
    path = anr.SecondaryPath(FS, 0.003)
    cases = [
        ("sprocket phase reference", dict(ref="sprocket")),
        ("bus speed at 50 Hz", dict(ref="bus")),
        ("sprocket, speed scale error 1 %", dict(ref="sprocket", speed_err=0.01)),
        ("sprocket + 0.5 Hz unseen wander", dict(ref="sprocket", lw_hz=0.5)),
        ("sprocket + 2 Hz unseen wander", dict(ref="sprocket", lw_hz=2.0)),
    ]
    for lab, kw in cases:
        d, f0m, bb = scenario(6.0, bb_db=80.0, **kw)
        ys = []
        for mu in mus:
            r = anr.harmonic_fxlms(d, f0m, FS, path, 3, mu=mu)
            ys.append(np.nan if r["diverged"] else tonal_residual_db(d, r["e"], bb, 2 * FS))
        ax.semilogx(mus, ys, marker=".", label=lab)
        out[lab] = ys
    ax.set_xlabel("normalised step size mu")
    ax.set_ylabel("tonal attenuation (dB)")
    ax.set_title("Reference quality decides the tonal ANR", fontsize=9)
    ax.legend(fontsize=6.5)
    return save(fig, "s2_tracking.png"), {"mu": mus, **out}


def main():
    res = {}
    res["learning_fig"], res["learning"] = study_a()
    print("A", res["learning"])
    res["stability_fig"], res["stability"] = study_b()
    print("B done")
    res["latency_fig"], res["latency"] = study_c()
    print("C", np.round(res["latency"]["tonal_db"], 1), np.round(res["latency"]["broadband_bound_db"], 1))
    res["tracking_fig"], res["tracking"] = study_d()
    print("D", {k: np.round(v, 1) for k, v in res["tracking"].items() if k != "mu"})
    dump("s2_anr.json", res)


if __name__ == "__main__":
    main()
