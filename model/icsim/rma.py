"""Reliability, maintainability, availability (Wasson 2016 ch. 34).

Parts-count prediction after MIL-HDBK-217F Notice 2 Appendix A, Ground Mobile
(GM) generic failure rates [R34] (V).  pi_Q = 1 for every part (assumption A:
the quality grade is not chosen yet); the sensitivity to that choice is shown.
The parts lists are estimates of a crew box and a CCU (A).
"""

from __future__ import annotations

import math

# generic failure rates, failures per 1e6 h, GM column [R34] (V)
LG = {
    "mcu32": 0.49,  # MOS microprocessor up to 32 bit
    "lin_1k_10k": 0.18,  # MOS linear 1001-10,000 transistors (codec, PHY)
    "lin_301_1k": 0.11,  # MOS linear 301-1000 (DC-DC controller)
    "lin_101_300": 0.065,  # MOS linear 101-300 (op-amp, RS-485 transceiver)
    "res_chip": 0.07,
    "cap_cer_chip": 0.13,
    "cap_tant_chip": 0.0036,
    "cap_alu": 0.047,
    "xfmr_switch": 0.0090,
    "xfmr_audio": 0.22,
    "coil": 0.00047,
    "relay_gp": 1.0,
    "conn_circ": 0.011,
    "sw_rotary": 2.0,
    "sw_push": 1.8,
}

# Crew box split into the digital path and the independent analog emergency path.
BOX_DIGITAL = {
    "mcu32": 1,
    "lin_1k_10k": 1,
    "lin_101_300": 4,
    "lin_301_1k": 1,
    "res_chip": 60,
    "cap_cer_chip": 45,
    "cap_tant_chip": 4,
    "cap_alu": 2,
    "xfmr_switch": 1,
    "coil": 2,
    "sw_rotary": 2,
    "conn_circ": 1,
}
BOX_EMERG = {  # relay, emergency amplifier, line transformer, PTT switch, its own supply filter
    "relay_gp": 1,
    "lin_101_300": 2,
    "xfmr_audio": 1,
    "res_chip": 20,
    "cap_cer_chip": 12,
    "cap_alu": 2,
    "sw_push": 1,
    "conn_circ": 1,
}
CCU = {
    "mcu32": 1,
    "lin_1k_10k": 3,
    "lin_101_300": 12,
    "lin_301_1k": 2,
    "res_chip": 150,
    "cap_cer_chip": 110,
    "cap_tant_chip": 8,
    "cap_alu": 6,
    "xfmr_switch": 2,
    "coil": 4,
    "conn_circ": 8,
}
SWITCH_TSN = {
    "mcu32": 1,
    "lin_1k_10k": 8,
    "lin_301_1k": 2,
    "res_chip": 120,
    "cap_cer_chip": 120,
    "cap_alu": 4,
    "xfmr_switch": 1,
    "xfmr_audio": 8,
    "coil": 4,
    "conn_circ": 8,
}
LINK = {"conn_circ": 2}  # one cable: two connectors; the wire itself is not modelled
ANALOG_AMP = {
    "lin_101_300": 3,
    "res_chip": 25,
    "cap_cer_chip": 15,
    "cap_alu": 2,
    "xfmr_audio": 1,
    "sw_rotary": 1,
    "sw_push": 1,
    "conn_circ": 1,
}


def lam(parts, pi_q=1.0):
    """Failure rate in failures per hour."""
    return sum(LG[k] * n for k, n in parts.items()) * pi_q * 1e-6


def contributors(parts):
    tot = sum(LG[k] * n for k, n in parts.items())
    return sorted(((k, n, LG[k] * n, 100.0 * LG[k] * n / tot) for k, n in parts.items()), key=lambda x: -x[2])


def r(rate, t):
    return math.exp(-rate * t)


def mission_reliability(arch, t, coverage=0.9, bypass=True):
    """Probability that the commander-driver voice path works for the whole mission t (h).

    A1 analog party line: both analog boxes, the line, and the central amplifier/power.
    A2 star + CCU; A3 switched network with distributed mixing.  With bypass, each box has
    an independent analog emergency path; a digital-path fault switches to it if detected
    (coverage c for internal box faults, 1.0 for CCU/link faults: the link supervision sees
    them by construction).
    """
    lb, le, lc, ll, ls = lam(BOX_DIGITAL), lam(BOX_EMERG), lam(CCU), lam(LINK), lam(SWITCH_TSN)
    la = lam(ANALOG_AMP)
    if arch == "A1":
        # boxes: analog amps; central amplifier ~ a CCU-sized analog unit (A: one ANALOG_AMP x4)
        return r(la, t) ** 2 * r(4 * la, t) * r(ll, t) ** 2
    if arch == "A2":
        central = r(lc, t) * r(ll, t) ** 2
    elif arch == "A3":
        central = r(ls, t) * r(ll, t) ** 2
    else:
        raise ValueError(arch)
    rd_box = r(lb, t)
    if not bypass:
        return rd_box**2 * central
    re = r(le, t)
    # each box: works digitally, or fails detected and the emergency path works
    # (but then BOTH must be in emergency mode to talk: the emergency line is shared;
    #  a box in bypass talks only to boxes in bypass, and the CCU commands all boxes to
    #  bypass when it sees one box lost: modelled as "system works if all digital, or all
    #  emergency paths work and the fault was detected")
    p_all_digital = rd_box**2 * central
    # central faults are always detected (link loss); box faults with probability coverage
    p_central_fault = 1 - central
    p_box_fault = 1 - rd_box**2
    det = (p_central_fault + p_box_fault * coverage) / max(p_central_fault + p_box_fault, 1e-15)
    p_fault_detected = (1 - p_all_digital) * det
    return p_all_digital + p_fault_detected * re**2


def mtbcf(arch, coverage=0.9, bypass=True):
    """Mean time to loss of the commander-driver voice path (h): integral of R(t)."""
    total, t, dt = 0.0, 0.0, 50.0
    while True:
        rt = mission_reliability(arch, t, coverage, bypass)
        total += rt * dt
        t += dt
        if rt < 1e-4 or t > 5e6:
            break
    return total


def availability(mtbf_h, mttr_h=0.5):
    """Inherent availability Ai = MTBF / (MTBF + MTTR)."""
    return mtbf_h / (mtbf_h + mttr_h)


# ---- FMEA (MIL-STD-882E severity 1..4 and probability A..E) --------------------------------
FMEA = [
    # item, failure mode, local effect, system effect, detection, compensation (mode), sev, prob
    (
        "CCU",
        "dead (power, processor)",
        "no downlink frames",
        "all boxes lose intercom/radio",
        "link supervision: no frame for 20 ms",
        "all boxes -> BYPASS, analog party line",
        2,
        "D",
    ),
    (
        "bus link",
        "open cable / connector",
        "one box loses frames",
        "that crew member isolated digitally",
        "link supervision",
        "box -> BYPASS; CCU sees missing uplink, commands others to keep digital",
        3,
        "D",
    ),
    (
        "bus link",
        "noise bursts (EMI)",
        "corrupted frames",
        "clicks, dropouts",
        "CRC-16, lost-frame counter",
        "concealment <= 3 frames, then mute; BYPASS if > 20 ms",
        4,
        "C",
    ),
    (
        "crew box",
        "processor hang",
        "relay drive stops toggling",
        "box silent",
        "dynamic relay drive (charge pump)",
        "relay drops -> BYPASS within relay release time",
        3,
        "D",
    ),
    (
        "crew box",
        "codec failure",
        "no audio to/from headset",
        "crew member loses digital path",
        "BIT: codec loop-back",
        "latched BYPASS",
        3,
        "D",
    ),
    (
        "crew box",
        "local supply out of range",
        "erratic audio",
        "as above",
        "BIT: supply monitor",
        "latched BYPASS",
        3,
        "D",
    ),
    (
        "crew box",
        "ANR divergence (wrong S_hat)",
        "loud low-frequency tone",
        "hearing hazard, masking",
        "error-power guard (+6 dB for 0.2 s), weight guard",
        "ANR off -> DEGRADED, output clamped",
        3,
        "D",
    ),
    (
        "crew box",
        "bypass relay stuck energised",
        "no emergency path",
        "latent until needed",
        "maintenance test of bypass (MAINT)",
        "none in operation: latent failure",
        2,
        "E",
    ),
    (
        "crew box",
        "emergency amplifier dead",
        "no audio in BYPASS",
        "latent until needed",
        "maintenance test of bypass",
        "none in operation: latent failure",
        3,
        "D",
    ),
    (
        "headset",
        "mic open / short",
        "no transmit",
        "crew member cannot talk",
        "BIT: mic bias current",
        "report to crew/CCU; spare headset",
        3,
        "C",
    ),
    (
        "headset",
        "earphone open",
        "no receive",
        "crew member cannot hear",
        "BIT: earphone impedance",
        "spare headset",
        3,
        "C",
    ),
    (
        "power 28 V",
        "1275E surge 100 V / 50 ms",
        "input stage stress",
        "box reset if unprotected",
        "design: surge stopper, see power study",
        "rides through",
        3,
        "C",
    ),
    (
        "power 28 V",
        "cranking dip to 16 V / 30 s; IES 12 V / 1 s",
        "supply droop",
        "reset during engine start",
        "design: wide-input converter + hold-up",
        "rides through",
        3,
        "B",
    ),
]


def risk(sev, prob):
    """MIL-STD-882E Table III risk assessment matrix -> High/Serious/Medium/Low."""
    table = {
        1: {"A": "High", "B": "High", "C": "High", "D": "Serious", "E": "Medium"},
        2: {"A": "High", "B": "High", "C": "Serious", "D": "Medium", "E": "Medium"},
        3: {"A": "Serious", "B": "Serious", "C": "Medium", "D": "Medium", "E": "Low"},
        4: {"A": "Medium", "B": "Medium", "C": "Low", "D": "Low", "E": "Low"},
    }
    return table[sev][prob]
