"""Study S5 - analysis of alternatives (Wasson 2016 ch. 32): A1 analog party line,
A2 digital star (RS-485 TDM, CCU mixing), A3 audio over switched Ethernet/TSN
with distributed mixing.  Both digital options carry the analog emergency
bypass.

Weights: 1-10 importance ratings, auto-normalised (Wasson's preferred method,
p. 694, instead of a 100-point split).  Measured criteria use linear utility
between a worst-acceptable and a best value; judged criteria use a 1-10 score
with the reason written next to it.  Sensitivity: every weight +/-10 % (Wasson
p. 696) and 20 000 random weight vectors (Dirichlet around the nominal).
Outputs: docs/img/s5_aoa.png, results/s5_aoa.json
"""

from __future__ import annotations

import json
import os

import numpy as np
from common import RES, dump, plt, save  # noqa: F401
from icsim import budget, chain, data, rma

ALTS = ["A1", "A2", "A3"]
NAMES = {
    "A1": "A1 analog party line",
    "A2": "A2 digital star (RS-485 TDM)",
    "A3": "A3 TSN, distributed mixing",
}


def measured():
    # fail-operational: loss of commander-driver voice per 1e6 12-h missions (S6)
    loss = {
        "A1": rma.mission_reliability("A1", 12.0, 0.9, False),
        "A2": rma.mission_reliability("A2", 12.0, 0.9, True),
        "A3": rma.mission_reliability("A3", 12.0, 0.9, True),
    }
    loss = {k: (1 - v) * 1e6 for k, v in loss.items()}
    # latency: worst-case crew-to-crew delay (S4); analog = 0
    lat = {a: budget.total(budget.intercom_path(a)) for a in ALTS}
    # intelligibility: best STI, tracked vehicle, ANR (S1). The analog party line sums every
    # open microphone: with 4 live mics the transmitted noise is +6 dB (A: no gating).
    base = chain.Config("x", anr_bb=True)
    sti_dig = chain.best_operating_point(base)["sti_max"]
    sti_ana = chain.best_operating_point(
        chain.Config("x", anr_bb=True, codec="wb", mic_rej=data.MIC_REJ - 6.0)
    )["sti_max"]
    sti = {"A1": sti_ana, "A2": sti_dig, "A3": sti_dig}
    # harness: conductor-metres for 6 crew stations (layout assumption A): average run 3 m
    runs = 6 * 3.0
    cond = {
        "A1": runs * 2 * 6
        + 6 * 2 * 2,  # 6 balanced pairs daisy-chained (icom, 2 radios, alarm, PTT, emerg) + power
        "A2": runs * (4 + 2 + 2),  # RS-485 2 pairs + power pair + emergency pair
        "A3": runs * (4 + 2 + 2),
    }  # 100BASE-TX 2 pairs + power + emergency pair
    return {"loss_per_1e6": loss, "latency_ms": lat, "sti": sti, "conductor_m": cond}


def utility(x, worst, best):
    u = (np.asarray(x, float) - worst) / (best - worst)
    return float(np.clip(u, 0.0, 1.0)) * 9.0 + 1.0  # 1..10


JUDGED = {
    # criterion: {alt: (score 1-10, reason)}
    "EMC robustness": {
        "A1": (4, "audio-band lines: bulk-current injection (CS114/CS116) is directly audible as hum/clicks"),
        "A2": (8, "differential RS-485, CRC rejects corrupted frames, concealment hides single losses"),
        "A3": (7, "transformer-isolated PHYs, CRC; faster edges raise RE102 emission risk"),
    },
    "growth": {
        "A1": (3, "every new source needs new pairs and a new party-line"),
        "A2": (6, "new channels fit in the TDM frame up to the link rate; CCU ports limit stations"),
        "A3": (9, "streams and stations added by configuration; data and video share the network"),
    },
    "cost and complexity": {
        "A1": (8, "few parts, no software"),
        "A2": (6, "custom TDM protocol, simple PHYs, one CCU"),
        "A3": (4, "TSN switch, time sync (gPTP), stream reservation: most software and parts"),
    },
}

WEIGHTS = {
    "fail-operational": 10,
    "intelligibility": 9,
    "latency": 7,
    "EMC robustness": 7,
    "harness mass": 5,
    "growth": 4,
    "cost and complexity": 5,
}


def scores(m):
    s = {
        "fail-operational": {a: utility(m["loss_per_1e6"][a], 1000.0, 10.0) for a in ALTS},
        "intelligibility": {
            a: utility(m["sti"][a], data.STI_THRESHOLD - 0.05, data.STI_OBJECTIVE) for a in ALTS
        },
        "latency": {a: utility(m["latency_ms"][a], 20.0, 0.0) for a in ALTS},
        "harness mass": {a: utility(m["conductor_m"][a], 300.0, 100.0) for a in ALTS},
    }
    for k, v in JUDGED.items():
        s[k] = {a: float(v[a][0]) for a in ALTS}
    return s


def total(s, w):
    wn = {k: v / sum(w.values()) for k, v in w.items()}
    return {a: sum(wn[k] * s[k][a] for k in w) for a in ALTS}


def main():
    m = measured()
    s = scores(m)
    tot = total(s, WEIGHTS)
    # +/-10 % one-at-a-time
    oat = {}
    for k in WEIGHTS:
        for f in (0.9, 1.1):
            w = dict(WEIGHTS)
            w[k] = WEIGHTS[k] * f
            t = total(s, w)
            oat["%s x%.1f" % (k, f)] = max(t, key=t.get)
    # Monte Carlo weights (Dirichlet, concentration 20 x nominal share)
    rng = np.random.default_rng(7)
    keys = list(WEIGHTS)
    base = np.array([WEIGHTS[k] for k in keys], float)
    wins = {a: 0 for a in ALTS}
    draws = rng.dirichlet(base / base.sum() * 20.0, size=20000)
    for wv in draws:
        t = total(s, dict(zip(keys, wv)))
        wins[max(t, key=t.get)] += 1
    win_pct = {a: 100.0 * n / len(draws) for a, n in wins.items()}
    res = {
        "measured": m,
        "scores": s,
        "weights": WEIGHTS,
        "total": tot,
        "oat_winner": oat,
        "mc_win_pct": win_pct,
        "judged_reasons": {k: {a: v[a][1] for a in ALTS} for k, v in JUDGED.items()},
    }

    fig, axs = plt.subplots(1, 2, figsize=(9, 3.4))
    crit = list(WEIGHTS)
    x = np.arange(len(crit))
    for i, a in enumerate(ALTS):
        axs[0].bar(x + (i - 1) * 0.27, [s[c][a] for c in crit], 0.27, label=NAMES[a])
    short = {"fail-operational": "fail-\noperational", "intelligibility": "intelli-\ngibility"}
    axs[0].set_xticks(x, [short.get(c, c.replace(" ", "\n")) for c in crit], fontsize=6.5)
    axs[0].set_ylabel("utility score (1-10)")
    axs[0].set_title(
        "Scores per criterion (weights: %s)" % "/".join(str(WEIGHTS[c]) for c in crit), fontsize=8
    )
    axs[0].legend(fontsize=7, loc="upper center", bbox_to_anchor=(0.5, -0.28), ncol=3)
    axs[1].bar(ALTS, [tot[a] for a in ALTS], color=["#4f5d75", "#2a6f97", "#66a182"])
    for i, a in enumerate(ALTS):
        axs[1].text(i, tot[a] + 0.1, "%.2f\nwins %.0f %%" % (tot[a], win_pct[a]), ha="center", fontsize=7)
    axs[1].set_ylim(0, 10)
    axs[1].set_ylabel("weighted score")
    axs[1].set_title("Total; share of 20 000 random weightings won", fontsize=8)
    axs[1].tick_params(axis="x", labelsize=7)
    res["fig"] = save(fig, "s5_aoa.png")
    dump("s5_aoa.json", res)
    print(
        json.dumps(
            {
                "measured": {k: {a: round(v, 3) for a, v in d.items()} for k, d in m.items()},
                "total": {a: round(v, 2) for a, v in tot.items()},
                "mc": win_pct,
                "oat_flips": {k: v for k, v in oat.items() if v != max(tot, key=tot.get)},
            },
            indent=1,
        )
    )
    _ = os


if __name__ == "__main__":
    main()
