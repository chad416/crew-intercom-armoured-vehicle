"""Study S4 - latency budgets for sidetone and crew-to-crew audio, per architecture,
plus the frame-length sensitivity.  Outputs: docs/img/s4_budget.png, results/s4_budget.json
"""

from __future__ import annotations

import numpy as np

from common import plt, save, dump  # noqa: F401
from icsim import budget

LIM = {k: v[0] for k, v in budget.LIMITS_MS.items()}


def main():
    res = {"limits": budget.LIMITS_MS}
    res["sidetone"] = budget.table(budget.sidetone_path(), LIM["sidetone"])
    for arch in ("A1", "A2", "A3"):
        res["intercom_" + arch] = budget.table(budget.intercom_path(arch), LIM["intercom"])
    # frame length sensitivity (A2, A3, sidetone)
    frames = [0.5, 1.0, 2.0, 4.0, 8.0]
    sens = {"frame_ms": frames}
    for key, fn in (
        ("sidetone", lambda f: budget.sidetone_path(frame_ms=f)),
        ("A2", lambda f: budget.intercom_path("A2", frame_ms=f)),
        ("A3", lambda f: budget.intercom_path("A3", frame_ms=f)),
    ):
        sens[key] = [round(budget.total(fn(f)), 3) for f in frames]
    res["frame_sensitivity"] = sens
    # converter filter choice at 16 kHz vs 48 kHz
    res["converters"] = {
        "16k_A": round(sum(e.ms for e in budget.converters(16000.0, "A")), 3),
        "48k_A": round(sum(e.ms for e in budget.converters(48000.0, "A")), 3),
        "192k_C": round(sum(e.ms for e in budget.converters(192000.0, "C")), 3),
    }

    fig, axs = plt.subplots(1, 2, figsize=(9, 3.4))
    paths = [("sidetone (local)", budget.sidetone_path(), LIM["sidetone"]),
             ("A2 star TDM", budget.intercom_path("A2"), LIM["intercom"]),
             ("A3 TSN, distributed mix", budget.intercom_path("A3"), LIM["intercom"])]
    for i, (lab, el, lim) in enumerate(paths):
        left = 0.0
        for j, e in enumerate(el):
            axs[0].barh(i, e.ms, left=left, color=plt.rcParams["axes.prop_cycle"].by_key()["color"][j % 7],
                        edgecolor="w", lw=0.5)
            left += e.ms
        axs[0].plot([lim, lim], [i - 0.4, i + 0.4], color="k", lw=1.5)
        axs[0].text(left + 0.2, i, "%.1f ms (limit %g)" % (left, lim), va="center", fontsize=7)
    axs[0].set_yticks(range(3), [p[0] for p in paths])
    axs[0].set_xlabel("worst-case delay (ms)")
    axs[0].set_xlim(0, 24)
    axs[0].set_title("Budgets at 16 kHz, 1 ms frames (bars = elements)", fontsize=9)
    for key, lab in (("sidetone", "sidetone"), ("A2", "A2 crew-to-crew"), ("A3", "A3 crew-to-crew")):
        axs[1].plot(frames, sens[key], marker="o", label=lab)
    axs[1].axhline(LIM["sidetone"], color="k", ls=":", lw=0.8)
    axs[1].axhline(LIM["intercom"], color="k", ls="--", lw=0.8)
    axs[1].set_xscale("log", base=2)
    axs[1].set_xticks(frames, [str(f) for f in frames])
    axs[1].set_xlabel("audio frame length (ms)")
    axs[1].set_ylabel("worst-case delay (ms)")
    axs[1].set_title("Frame length: 1 ms keeps both paths inside", fontsize=9)
    axs[1].legend(fontsize=7)
    res["fig"] = save(fig, "s4_budget.png")
    dump("s4_budget.json", res)
    for k in ("sidetone", "intercom_A1", "intercom_A2", "intercom_A3"):
        print(k, res[k]["total_ms"], "limit", res[k]["limit_ms"], "margin", res[k]["margin_pct"], "%")
    print(sens)
    print(res["converters"])
    _ = np


if __name__ == "__main__":
    main()
