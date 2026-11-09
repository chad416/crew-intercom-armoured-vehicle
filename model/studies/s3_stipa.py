"""Study S3 - model validation and measured STI gain (Wasson 2016 ch. 33: validate
the model against an independent method before using it).

A. The indirect STI (IEC 60268-16 Annex A, used by S1) against the direct
   STIPA method run on synthesised signals, at the operating point of S1 and
   at a range of SNRs.  Pass criterion: |difference| <= 0.03 (the STIPA
   repeatability seen in the clean-channel check).
B. Direct STIPA measurement THROUGH the time-varying adaptive filter: the ear
   signal = STIPA "speech" + transmitted noise + residual vehicle noise after the
   speed-referenced FxLMS (which also hears the speech at its error mic).  This is
   the measured STI gain of the tonal ANR, and a check that it does not cancel
   speech.
Outputs: docs/img/s3_stipa.png, results/s3_stipa.json
"""

from __future__ import annotations

import numpy as np
from common import dump, plt, save  # noqa: F401
from icsim import anr, chain, data, sti

FS = 32000
SECONDS = 16.0
MU_32K = 0.00125


def shaped_noise(levels_db, n, rng, fs=FS):
    """Stationary Gaussian noise with the given octave-band levels (125..8k), Pa."""
    bands = sti.octave_split(rng.standard_normal(n), fs)
    out = np.zeros(n)
    for k in range(7):
        b = bands[k]
        out += b * (20e-6 * 10 ** (levels_db[k] / 20.0) / np.sqrt(np.mean(b * b)))
    return out


def part_a():
    rng = np.random.default_rng(11)
    c = chain.Config("C3", anr_bb=True, anr_tonal_db=20.0)
    op = chain.best_operating_point(c)
    r = chain.evaluate(c, op["speech_ear_dba"])
    s_ear = r["snr_ear"] + 0  # placeholder to keep the dict access explicit
    del s_ear
    # rebuild the band levels used inside evaluate()
    s_mic, n_mic = chain.mic_levels(c)
    bp = chain.band_pass_db(*chain.CODEC_BANDS[c.codec])[1:]
    g = op["speech_ear_dba"] - chain.a_level(np.concatenate([[-100.0], s_mic + bp]))
    s_lev = s_mic + bp + g
    n_lev = chain.db_sum(chain.ear_noise(c)[1:], n_mic + bp + g)
    rows = []
    for shift in (-12.0, -6.0, 0.0, 6.0, 12.0):
        for seed in (1, 2):
            rng = np.random.default_rng(100 + seed)
            n = int(FS * SECONDS)
            x = sti.stipa_signal(s_lev, FS, SECONDS, rng) + shaped_noise(n_lev - shift, n, rng)
            d, _, _ = sti.sti_direct(x, FS)
            i, _ = sti.sti_indirect(s_lev, n_lev - shift)
            rows.append({"snr_shift_db": shift, "seed": seed, "direct": d, "indirect": i, "diff": d - i})
    return {
        "operating_point_sti": op["sti"],
        "speech_ear_dba": op["speech_ear_dba"],
        "rows": rows,
        "s_lev": s_lev,
        "n_lev": n_lev,
    }


def part_b(a):
    """Tonal ANR in the loop: STI measured directly, ANR off vs on."""
    rng = np.random.default_rng(5)
    n = int(FS * SECONDS)
    c = chain.Config("C2", anr_bb=True)
    t = chain.tonal_share("tracked")
    s_mic, n_mic = chain.mic_levels(c)
    bp = chain.band_pass_db(*chain.CODEC_BANDS[c.codec])[1:]
    g = a["speech_ear_dba"] - chain.a_level(np.concatenate([[-100.0], s_mic + bp]))
    s_lev = s_mic + bp + g
    n_tx = n_mic + bp + g
    ear = chain.ear_noise(c)[1:]  # 125..8k, helmet + broadband ANR, tones included
    tshare = t[1:]
    bb_lev = ear + 10 * np.log10(np.maximum(1 - tshare, 1e-6))
    # tones: 60 km/h, pitch 0.149 m -> 111.9 Hz; H1 in the 125 band, H2 and H3 in the 250 band
    tone_125 = ear[0] + 10 * np.log10(max(tshare[0], 1e-6))
    tone_250 = ear[1] + 10 * np.log10(max(tshare[1], 1e-6)) - 3.0
    harm = [tone_125, tone_250, tone_250]
    speed = 60.0
    d_tone, f0, _ = anr.track_noise(FS, SECONDS, speed, data.TRACK_PITCH_M, harm, rng=rng)
    speech = sti.stipa_signal(s_lev, FS, SECONDS, rng)
    stat = shaped_noise(bb_lev, n, rng) + shaped_noise(n_tx, n, rng)
    out = {"tone_levels_db": harm, "f0_hz": float(f0[0])}
    x_off = speech + stat + d_tone
    out["off"] = sti.sti_direct(x_off, FS)[0]
    # ANR on: the error mic hears everything at the ear; the controller adds anti-noise
    path = anr.SecondaryPath(FS, 0.003)
    # mu scaled with the sample rate: 0.01 at 4 kHz (S2) -> 0.00125 at 32 kHz (same rate per second)
    r = anr.harmonic_fxlms(stat + d_tone + speech, f0, FS, path, 3, mu=MU_32K)
    x_on = r["e"]
    out["on"] = sti.sti_direct(x_on, FS, skip_s=2.0)[0]
    # what the anti-noise did to the speech: compare the speech band levels with/without
    lev_off = sti.band_levels(x_off[2 * FS :], FS)
    lev_on = sti.band_levels(x_on[2 * FS :], FS)
    out["band_level_change_db"] = lev_on - lev_off
    # indirect prediction for both cases (tones as noise power)
    n_off = chain.db_sum(ear, n_tx)
    tl = np.array([tone_125, 10 * np.log10(2 * 10 ** (tone_250 / 10))])
    resid = chain.db_sum(np.concatenate([[x for x in [None]] and [] or [], bb_lev]), n_tx)
    # tonal residual measured from the run (power of tones after ANR estimated from the band change)
    out["indirect_off"] = sti.sti_indirect(s_lev, n_off)[0]
    att = -(lev_on - lev_off)[:2]
    n_on = n_off.copy()
    for k in range(2):
        tone_p = 10 ** (tl[k] / 10)
        rest = 10 ** (n_off[k] / 10) - tone_p
        n_on[k] = 10 * np.log10(rest + tone_p * 10 ** (-max(att[k], 0) / 10) + 1e-30)
    out["indirect_on"] = sti.sti_indirect(s_lev, n_on)[0]
    _ = resid
    return out


def main():
    a = part_a()
    b = part_b(a)
    diffs = np.array([r["diff"] for r in a["rows"]])
    res = {"A": a, "B": b, "max_abs_diff": float(np.max(np.abs(diffs))), "mean_diff": float(diffs.mean())}
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.3))
    ind = np.array([r["indirect"] for r in a["rows"]])
    dirc = np.array([r["direct"] for r in a["rows"]])
    axs[0].plot([0, 1], [0, 1], color="k", lw=0.8)
    axs[0].fill_between([0, 1], [-0.03, 0.97], [0.03, 1.03], color="0.9")
    axs[0].scatter(ind, dirc, s=14)
    axs[0].set_xlim(0.1, 0.8)
    axs[0].set_ylim(0.1, 0.8)
    axs[0].set_xlabel("indirect STI (model used in S1)")
    axs[0].set_ylabel("direct STIPA on synthesised signals")
    axs[0].set_title("Validation: max |diff| %.3f (band: +/-0.03)" % res["max_abs_diff"], fontsize=9)
    labels = ["indirect\nANR off", "direct\nANR off", "direct\nANR on", "indirect\nANR on"]
    vals = [b["indirect_off"], b["off"], b["on"], b["indirect_on"]]
    axs[1].bar(labels, vals, color=["#9aa5b1", "#2a6f97", "#66a182", "#9aa5b1"])
    for i, v in enumerate(vals):
        axs[1].text(i, v + 0.005, "%.3f" % v, ha="center", fontsize=8)
    axs[1].set_ylim(0, max(vals) * 1.2)
    axs[1].set_ylabel("STI")
    axs[1].set_title("Tonal ANR in the loop, measured by STIPA", fontsize=9)
    res["fig"] = save(fig, "s3_stipa.png")
    dump("s3_stipa.json", res)
    print(
        "A max|diff| %.3f mean %.3f; op STI %.3f"
        % (res["max_abs_diff"], res["mean_diff"], a["operating_point_sti"])
    )
    for r in a["rows"]:
        print(
            "  shift %+5.1f seed %d direct %.3f indirect %.3f"
            % (r["snr_shift_db"], r["seed"], r["direct"], r["indirect"])
        )
    print("B", {k: (np.round(v, 3) if not isinstance(v, list) else v) for k, v in b.items()})


if __name__ == "__main__":
    main()
