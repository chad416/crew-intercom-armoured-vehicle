"""Study S1 - the speech chain: where is intelligibility lost, and what does it
take to stay intelligible at >= 110 dB(A) within the hearing limits?

Outputs: docs/img/s1_*.png, results/s1_chain.json
"""

from __future__ import annotations

from dataclasses import replace

import numpy as np

from common import plt, save, dump  # noqa: F401  (path setup)
from icsim import chain, data

CONFIGS = [
    ("C0 analog, helmet only, NB", dict(codec="nb")),
    ("C1 digital WB, helmet only", dict()),
    ("C2 C1 + broadband ANR", dict(anr_bb=True)),
    ("C3 C2 + tonal ANR (20 dB)", dict(anr_bb=True, anr_tonal_db=20.0)),
    ("C4 C3 + 32 kHz codec", dict(anr_bb=True, anr_tonal_db=20.0, codec="swb")),
    ("C5 C3 + mic +3 dB", dict(anr_bb=True, anr_tonal_db=20.0, mic_rej=data.MIC_REJ + 3.0)),
    ("C6 C5 + earplug +10 dB", dict(anr_bb=True, anr_tonal_db=20.0, mic_rej=data.MIC_REJ + 3.0,
                                    passive_extra_db=10.0)),
]


def fig_spectra():
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.4), sharey=True)
    for ax, v in zip(axs, ("tracked", "wheeled")):
        c0 = chain.Config("x", vehicle=v)
        x = np.arange(8)
        w = 0.2
        series = [
            ("cab (crew position)", chain.cab_noise(c0)),
            ("ear, helmet", chain.ear_noise(c0)),
            ("ear, + broadband ANR", chain.ear_noise(replace(c0, anr_bb=True))),
            ("ear, + tonal ANR", chain.ear_noise(replace(c0, anr_bb=True, anr_tonal_db=20.0))),
        ]
        for i, (lab, y) in enumerate(series):
            ax.bar(x + (i - 1.5) * w, y, w, label=lab)
        ax.set_xticks(x, ["63", "125", "250", "500", "1k", "2k", "4k", "8k"])
        ax.set_ylim(40, 130)
        ax.set_xlabel("octave band (Hz)")
        ax.set_title(data.NOISE[v]["label"], fontsize=9)
        lev = [chain.a_level(y) for _, y in series]
        ax.text(0.02, 0.97, "dB(A): " + " / ".join("%.1f" % v for v in lev), transform=ax.transAxes,
                va="top", fontsize=8)
    axs[0].set_ylabel("SPL (dB re 20 uPa)")
    axs[1].legend(fontsize=7, loc="upper right")
    return save(fig, "s1_spectra.png")


def fig_sti_vs_level(vehicle="tracked"):
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.5))
    levels = np.arange(70.0, 112.0, 1.0)
    out = {}
    for name, kw in CONFIGS:
        c = chain.Config(name, vehicle=vehicle, **kw)
        rows = [chain.evaluate(c, L) for L in levels]
        st = [r["sti"] for r in rows]
        tw = [r["twa_dba"] for r in rows]
        axs[0].plot(levels, st, label=name)
        axs[1].plot(levels, tw, label=name)
        out[name] = {"sti": st, "twa": tw}
    peak_lim = data.PEAK_AT_EAR_DB - data.SPEECH_CREST_DB
    for ax in axs:
        ax.axvspan(peak_lim, 112, color="0.85", lw=0)
        ax.set_xlabel("speech level at the ear (dB(A))")
    axs[0].axhline(data.STI_THRESHOLD, color="k", ls="--", lw=0.8)
    axs[0].axhline(data.STI_OBJECTIVE, color="k", ls=":", lw=0.8)
    axs[0].text(71, data.STI_THRESHOLD + 0.005, "threshold 0.41 (MRT 85 %)", fontsize=7)
    axs[0].text(71, data.STI_OBJECTIVE + 0.005, "objective 0.48 (MRT 90 %)", fontsize=7)
    axs[0].set_ylabel("STI")
    axs[0].set_ylim(0.1, 0.65)
    axs[1].axhline(data.TWA_LIMIT_DBA, color="k", ls="--", lw=0.8)
    axs[1].set_ylabel("8-h TWA at the ear (dB(A))")
    axs[1].legend(fontsize=6.5, loc="upper left")
    fig.suptitle("Tracked vehicle 111 dB(A): STI and noise dose against earphone level "
                 "(grey: above the 115 dB peak limit)", fontsize=9)
    return save(fig, "s1_sti_vs_level.png"), out


def fig_tx_heatmap():
    base = chain.Config("C3", anr_bb=True, anr_tonal_db=20.0)
    dists = np.array([3.0, 2.0, 1.5, 1.0, 0.8, 0.6])
    efforts = ["normal", "raised", "loud", "shout"]
    grid = np.zeros((len(efforts), len(dists)))
    for i, e in enumerate(efforts):
        for j, d in enumerate(dists):
            c = replace(base, effort=e, mic_near_gain_db=20 * np.log10(100.0 / d))
            grid[i, j] = chain.best_operating_point(c)["sti_max"]
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.3))
    im = axs[0].imshow(grid, cmap="viridis", vmin=0.2, vmax=0.75, aspect="auto")
    axs[0].set_xticks(range(len(dists)), ["%.1f" % d for d in dists])
    axs[0].set_yticks(range(len(efforts)), efforts)
    axs[0].set_xlabel("lip-to-microphone distance (cm)")
    axs[0].set_title("best reachable STI (tracked, C3)", fontsize=9)
    for i in range(len(efforts)):
        for j in range(len(dists)):
            axs[0].text(j, i, "%.2f" % grid[i, j], ha="center", va="center", fontsize=7,
                        color="w" if grid[i, j] < 0.5 else "k")
    fig.colorbar(im, ax=axs[0], fraction=0.04)
    s, n = chain.mic_levels(base)
    bands = ["125", "250", "500", "1k", "2k", "4k", "8k"]
    axs[1].bar(bands, s - n, color=[COL_NEG if v < 0 else COL_POS for v in (s - n)])
    axs[1].axhline(0, color="k", lw=0.6)
    axs[1].set_ylabel("transmit SNR at the mic (dB)")
    axs[1].set_xlabel("octave band (Hz)")
    axs[1].set_title("raised voice, 1.5 cm, pressure-gradient mic", fontsize=9)
    return save(fig, "s1_transmit.png"), {"dists_cm": dists, "efforts": efforts, "sti_max": grid,
                                          "tx_snr_raised_1p5cm": s - n}


COL_NEG = "#d1495b"
COL_POS = "#2a6f97"


def fig_vs_vehicle_level():
    shifts = np.arange(-20.0, 16.0, 2.0)
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.4), sharey=True)
    out = {}
    for ax, v in zip(axs, ("tracked", "wheeled")):
        for name, kw in CONFIGS:
            xs, ys = [], []
            for sh in shifts:
                c = chain.Config(name, vehicle=v, level_shift_db=float(sh), **kw)
                r = chain.best_operating_point(c)
                xs.append(chain.a_level(chain.cab_noise(c)))
                # best STI that also respects the 8-h TWA
                rows = [x for x in chain.sweep_level(c) if x["peak_db"] <= data.PEAK_AT_EAR_DB
                        and x["twa_dba"] < data.TWA_LIMIT_DBA]
                ys.append(max([x["sti"] for x in rows], default=np.nan))
            ax.plot(xs, ys, marker=".", label=name)
            out["%s|%s" % (v, name)] = {"dba": xs, "sti_dose_ok": ys}
        ax.axhline(data.STI_THRESHOLD, color="k", ls="--", lw=0.8)
        ax.axhline(data.STI_OBJECTIVE, color="k", ls=":", lw=0.8)
        ax.axvline(110, color="0.5", lw=0.8)
        ax.set_ylim(0.25, 0.95)
        ax.set_xlabel("vehicle noise at the crew position (dB(A))")
        ax.set_title(v + " spectrum shape", fontsize=9)
    axs[0].set_ylabel("best STI with TWA < 85 dB(A), 8 h")
    axs[1].legend(fontsize=6.5, loc="lower left")
    return save(fig, "s1_vs_vehicle.png"), out


def table_operating_points():
    rows = []
    for v in ("tracked", "wheeled"):
        for name, kw in CONFIGS:
            c = chain.Config(name, vehicle=v, **kw)
            r = chain.best_operating_point(c)
            m = chain.max_vehicle_level(c)
            # hours allowed at the operating point (equal-energy rule)
            hours = 8.0 * 10 ** ((data.TWA_LIMIT_DBA - r["twa_dba"]) / 10.0)
            rows.append({
                "vehicle": v, "config": name, "noise_ear_dba": r["noise_ear_dba"],
                "sti_max": r["sti_max"], "reached_0p41": r["reached"], "speech_ear_dba": r["speech_ear_dba"],
                "twa_8h": r["twa_dba"], "hours_allowed": hours,
                "max_vehicle_dba_0p41_dose_ok": None if m is None else m[0],
            })
    return rows


def main():
    res = {"spectra_fig": fig_spectra()}
    res["sti_vs_level_fig"], res["sti_vs_level"] = fig_sti_vs_level()
    res["transmit_fig"], res["transmit"] = fig_tx_heatmap()
    res["vs_vehicle_fig"], res["vs_vehicle"] = fig_vs_vehicle_level()
    res["table"] = table_operating_points()
    res["tonal_share"] = {v: chain.tonal_share(v) for v in ("tracked", "wheeled")}
    res["cab_dba"] = {v: chain.a_level(data.NOISE[v]["oct"]) for v in ("tracked", "wheeled")}
    dump("s1_chain.json", res)
    for r in res["table"]:
        print("%-8s %-28s ear %5.1f  STImax %.3f  hit %-5s  L %5.1f  TWA %5.1f  h %5.1f  maxveh %s" % (
            r["vehicle"], r["config"], r["noise_ear_dba"], r["sti_max"], r["reached_0p41"], r["speech_ear_dba"],
            r["twa_8h"], r["hours_allowed"], r["max_vehicle_dba_0p41_dose_ok"] and round(r["max_vehicle_dba_0p41_dose_ok"], 1)))


if __name__ == "__main__":
    main()
