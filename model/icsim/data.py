"""Sourced input data.  Every array carries the reference key used in
DOSSIER.md §14 (reference list) and in docs/sources.md, and its status:
V = read in the source, G = read off a published graph (+/-2 dB),
D = derived from V/G values by the stated method, A = assumption.

Octave bands are 63, 125, 250, 500, 1k, 2k, 4k, 8k Hz (index 0..7); the STI
uses 125..8k (index 1..7).
"""

from __future__ import annotations

import numpy as np

OCT = np.array([63.0, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0])
A_WEIGHT = np.array([-26.2, -16.1, -8.6, -3.2, 0.0, 1.2, 1.0, -1.1])  # IEC 61672-1 octave values

# ---- vehicle interior noise at a crew position (dB SPL, linear) ----------------------------
# [R2] Buck, Hamery & Zimpfer, ICA 2010, Fig. 1, 1/3-octave levels read from the graph (G),
# summed to octaves (D).  Check: A-weighted sums 111.3 / 110.1 dB(A) vs 111 / 110 published.
NOISE = {
    "tracked": {
        "label": "tracked vehicle, 60 km/h (125 dB lin, 111 dB(A))",
        "oct": np.array([121.2, 121.3, 115.6, 106.3, 101.5, 99.1, 93.7, 86.7]),
        "ref": "R2",
        "speed_kmh": 60.0,
        # 1/3-octave bands 80/100 Hz (121/120 dB) stand 14-19 dB above their neighbours:
        # the track-laying tone (f0 = v/p, [R1] Sec. 5).  Tonal share per octave (D, see
        # chain.tonal_share): energy above the smooth broadband trend.
    },
    "wheeled": {
        "label": "wheeled vehicle, 60 km/h (115 dB lin, 110 dB(A))",
        "oct": np.array([96.7, 97.5, 108.8, 106.8, 107.4, 98.4, 95.0, 82.4]),
        "ref": "R2",
        "speed_kmh": 60.0,
    },
}

# 1/3-octave source values [R2] Fig. 1 (G) -- kept for the tonal-share analysis
THIRD_F = np.array(
    [50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000,
     5000, 6300, 8000, 10000], dtype=float)
THIRD = {
    "tracked": np.array([98, 102, 121, 120, 107, 114, 114, 109, 102, 101, 101, 102, 98, 96, 95, 95, 94, 94, 91,
                         88, 86, 84, 81, 78], dtype=float),
    "wheeled": np.array([94, 88, 92, 93, 90, 94, 95, 108, 95, 94, 102, 105, 102, 105, 97, 93, 95, 92, 94, 87, 82,
                         80, 77, 72], dtype=float),
}

# Track pitch [R1] Sec. 5: 0.149 m (tracked APC); f0 = 74.6 Hz at 40 km/h (V).
TRACK_PITCH_M = 0.149

# ---- hearing protection ---------------------------------------------------------------------
# Crew helmet real-ear attenuation, 125..8k [R17] MIL-DTL-44117B Table I (V, secondary copy).
# 63 Hz is not in the table: A = 125 Hz value minus 2 dB (passive attenuation falls towards LF).
HELMET_ATT = np.array([12.0, 14.0, 16.0, 21.0, 23.0, 28.0, 35.0, 30.0])
# Broadband (analog feedback) ANR insertion gain of a prototype ANR headset [R3] Fig. 7 (G),
# 1/3 octaves energy-averaged to octaves (D).  Negative = amplification (1-2 kHz).
ANR_BB = np.array([16.5, 16.0, 13.0, 2.5, -4.0, -1.0, 0.0, 0.0])

# ---- speech -----------------------------------------------------------------------------------
# Octave-band speech level at 1 m, ANSI S3.5-1997 Table 4 spectrum levels + 10 log10(0.707 fc)
# [R14] (V secondary, D).  250..8k; the 125 Hz band is not in ANSI S3.5: taken as the 250 Hz
# value minus 3 dB, from the IEC 60268-16 Table A.4 male shape (-2.5 / +0.5 dB) [R11] (D).
_SPEECH_250_8K = {
    "normal": [57.2, 59.8, 53.5, 48.8, 43.8, 38.6],
    "raised": [61.5, 65.7, 62.4, 56.8, 51.3, 42.6],
    "loud": [64.0, 70.4, 70.7, 65.9, 59.9, 48.9],
    "shout": [65.0, 74.8, 79.8, 75.8, 68.9, 58.2],
}
SPEECH_1M = {k: np.array([v[0] - 3.0] + v) for k, v in _SPEECH_250_8K.items()}  # 125..8k

# Lombard slope above ~43 dB(A): 0.65 dB/dB [R15] (V).

# ---- microphone ---------------------------------------------------------------------------
# Pressure-gradient boom microphone far-field rejection relative to close speech: crossover
# 2.5 kHz, -6 dB/octave below [R18] (V, vendor) -> rejection per octave 125..8k (D).
MIC_REJ = np.array([max(0.0, 6.0 * np.log2(2500.0 / f)) for f in OCT[1:]])
# Lip-to-mic distance 1.5 cm: near-field speech gain over 1 m = 20 log10(100/1.5) = 36.5 dB (A).
MIC_NEAR_GAIN_DB = 20.0 * np.log10(100.0 / 1.5)

# ---- requirements sources -----------------------------------------------------------------------
# MIL-STD-1474E Table I, MRT -> STI [R4] (V)
MRT_STI = {95: 0.57, 90: 0.48, 85: 0.41, 80: 0.36, 75: 0.32}
STI_THRESHOLD = 0.41  # MRT 85 %: MIL-STD-1474E 4.3.1 prediction criterion [R4]
STI_OBJECTIVE = 0.48  # MRT 90 %: ~ MIL-STD-1472G Table XX "normal acceptable" (91 % MRT) [R5]
TWA_LIMIT_DBA = 85.0  # MIL-STD-1474E 4.2.3.1 c: 8-h TWA at the ear with protection [R4]
PEAK_AT_EAR_DB = 115.0  # MIL-STD-1472G 5.3.1.8.2 a(3): max peak voice level at the ear [R5]
SPEECH_CREST_DB = 12.0  # A: peak-to-rms of running speech (assumption, used for the 115 dB peak limit)
ABOVE_AMBIENT_DB = 15.0  # MIL-STD-1472G 5.3.1.8.2 a(2): comms output >= 15 dB above ambient [R5]

# ---- converters (latency) -------------------------------------------------------------------------
# TI SLAA557 TLV320AIC3204 application reference guide: decimation/interpolation filter group
# delays in sample periods [R37] (V).
ADC_GD = {"A": 17, "B": 11, "C": 11}
DAC_GD = {"A": 21, "B": 18, "C": 13}
