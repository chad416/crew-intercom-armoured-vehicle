"""Study S6 - RMA: parts-count failure rates, mission reliability of the
commander-driver voice path per architecture with and without the analog
emergency bypass, BIT-coverage sensitivity, availability, FMEA table.
Outputs: docs/img/s6_rma.png, results/s6_rma.json, results/fmea.md
"""

from __future__ import annotations

import numpy as np

from common import RES, plt, save, dump  # noqa: F401
from icsim import rma

MISSION_H = 12.0  # assumption: one 12 h operating period


def main():
    res = {"mission_h": MISSION_H}
    units = {"crew box, digital path": rma.BOX_DIGITAL, "crew box, emergency path": rma.BOX_EMERG,
             "CCU": rma.CCU, "TSN switch": rma.SWITCH_TSN, "analog box amplifier": rma.ANALOG_AMP}
    res["units"] = {}
    for name, parts in units.items():
        lam = rma.lam(parts)
        res["units"][name] = {"lambda_per_1e6h": lam * 1e6, "mtbf_h": 1.0 / lam,
                              "top3": [(k, n, round(x, 3), round(p, 1)) for k, n, x, p in rma.contributors(parts)[:3]]}
    box_l = rma.lam(rma.BOX_DIGITAL) + rma.lam(rma.BOX_EMERG)
    res["crew_box_mtbf_h"] = 1.0 / box_l
    res["crew_box_ai"] = rma.availability(1.0 / box_l, 0.5)

    cases = [("A1 analog party line", "A1", False), ("A2 star, no bypass", "A2", False),
             ("A2 star + bypass", "A2", True), ("A3 TSN, no bypass", "A3", False), ("A3 TSN + bypass", "A3", True)]
    res["mission"] = {}
    for lab, arch, byp in cases:
        rel = rma.mission_reliability(arch, MISSION_H, 0.9, byp)
        res["mission"][lab] = {"R_mission": rel, "unrel_per_1e6_missions": (1 - rel) * 1e6,
                               "mtbcf_h": rma.mtbcf(arch, 0.9, byp)}
    cov = np.linspace(0.5, 1.0, 11)
    res["coverage"] = {"c": cov, "A2_bypass_unrel_1e6": [(1 - rma.mission_reliability("A2", MISSION_H, c, True)) * 1e6
                                                         for c in cov]}
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.3))
    labs = [c[0] for c in cases]
    vals = [res["mission"][l]["unrel_per_1e6_missions"] for l in labs]
    axs[0].barh(labs, vals, color=["#4f5d75", "#d1495b", "#2a6f97", "#d1495b", "#2a6f97"])
    axs[0].set_xscale("log")
    axs[0].set_xlabel("loss of commander-driver voice per 10^6 missions (12 h)")
    for i, v in enumerate(vals):
        axs[0].text(v * 1.1, i, "%.0f" % v, va="center", fontsize=7)
    axs[1].plot(cov, res["coverage"]["A2_bypass_unrel_1e6"], marker="o")
    axs[1].set_xlabel("BIT coverage of internal crew-box faults")
    axs[1].set_ylabel("loss per 10^6 missions (A2 + bypass)")
    axs[1].set_title("Bypass is only as good as fault detection", fontsize=9)
    res["fig"] = save(fig, "s6_rma.png")

    # FMEA markdown (Wasson Fig. 34.20 columns, MIL-STD-882E severity/probability)
    lines = ["| # | Item | Failure mode | Local effect | System effect | Detection | Compensating provision "
             "| Sev | Prob | Risk |", "|---|---|---|---|---|---|---|---|---|---|"]
    for i, (item, mode, loc, sysf, det, comp, sev, prob) in enumerate(rma.FMEA, 1):
        lines.append("| %d | %s | %s | %s | %s | %s | %s | %d | %s | %s |" % (
            i, item, mode, loc, sysf, det, comp, sev, prob, rma.risk(sev, prob)))
    with open(RES + "/fmea.md", "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    dump("s6_rma.json", res)
    for k, v in res["units"].items():
        print("%-26s lambda %.2f /1e6 h  MTBF %.0f h  top %s" % (k, v["lambda_per_1e6h"], v["mtbf_h"], v["top3"]))
    print("crew box MTBF %.0f h, Ai %.6f" % (res["crew_box_mtbf_h"], res["crew_box_ai"]))
    for k, v in res["mission"].items():
        print("%-22s R %.6f  loss/1e6 %.0f  MTBCF %.0f h" % (k, v["R_mission"], v["unrel_per_1e6_missions"], v["mtbcf_h"]))


if __name__ == "__main__":
    main()
