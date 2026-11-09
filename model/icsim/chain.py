"""End-to-end speech chain: talker -> boom microphone -> codec -> bus -> earphone
-> ear, with the vehicle noise reaching the ear through the helmet (and ANR)
and reaching the microphone directly.  Computes the STI at the listener and
the hearing-exposure constraints of MIL-STD-1474E / MIL-STD-1472G.

Octave index convention: arrays of 8 are 63..8k Hz (data.OCT); the STI uses
the last 7 (125..8k).
"""

from __future__ import annotations

from dataclasses import dataclass, field, replace
from functools import lru_cache

import numpy as np

from . import data, sti

CODEC_BANDS = {  # passband edges (Hz) of the speech path
    "nb": (300.0, 3400.0),  # narrowband telephony / legacy analog intercom
    "wb": (50.0, 7000.0),  # 16 kHz sampling
    "swb": (50.0, 14000.0),  # 32 kHz sampling
}


def db_sum(*levels):
    return 10.0 * np.log10(sum(10.0 ** (np.asarray(x, dtype=float) / 10.0) for x in levels))


def a_level(oct_db):
    """Overall A-weighted level of an 8-band octave spectrum (63..8k)."""
    return float(10.0 * np.log10(np.sum(10.0 ** ((np.asarray(oct_db) + data.A_WEIGHT) / 10.0))))


def lin_level(oct_db):
    return float(10.0 * np.log10(np.sum(10.0 ** (np.asarray(oct_db) / 10.0))))


def band_pass_db(lo, hi, f=data.OCT):
    """Level change (dB) per octave band for an ideal passband [lo, hi], assuming the
    energy is spread evenly over log-frequency inside each octave."""
    out = []
    for fc in f:
        a, b = fc / np.sqrt(2.0), fc * np.sqrt(2.0)
        x0, x1 = max(a, lo), min(b, hi)
        frac = np.log2(x1 / x0) if x1 > x0 else 0.0
        out.append(10.0 * np.log10(max(frac, 1e-10)))
    return np.array(out)


@lru_cache(maxsize=None)
def _tonal_share(vehicle):
    """Fraction of each octave's energy that is tonal, from the 1/3-octave spectrum:
    energy above the running median (5 bands) of the spectrum, where the band exceeds it
    by more than 3 dB.  Returns 8 values (63..8k)."""
    third = data.THIRD[vehicle]
    f = data.THIRD_F
    trend = np.array([np.median(third[max(0, i - 2) : i + 3]) for i in range(len(third))])
    e = 10.0 ** (third / 10.0)
    ex = np.where(third > trend + 3.0, e - 10.0 ** (trend / 10.0), 0.0)
    out = []
    for fc in data.OCT:
        sel = (f >= fc / np.sqrt(2.0) * 0.99) & (f <= fc * np.sqrt(2.0) * 1.01)
        out.append(float(ex[sel].sum() / e[sel].sum()) if sel.any() else 0.0)
    return tuple(out)


def tonal_share(vehicle):
    return np.array(_tonal_share(vehicle))


@dataclass
class Config:
    name: str
    vehicle: str = "tracked"
    level_shift_db: float = 0.0  # move the whole vehicle spectrum up/down
    helmet: bool = True
    passive_extra_db: float = 0.0  # extra passive attenuation, e.g. a communication earplug (A)
    anr_bb: bool = False  # broadband (analog feedback) ANR, [R3] curve
    anr_tonal_db: float = 0.0  # attenuation of the tonal part by the speed-referenced ANR
    codec: str = "wb"
    effort: str = "raised"  # MIL-STD-1474E 4.3.1 assumes a raised voice
    mic_near_gain_db: float = data.MIC_NEAR_GAIN_DB
    mic_rej: np.ndarray = field(default_factory=lambda: data.MIC_REJ.copy())
    speech_duty: float = 0.3  # fraction of the shift the earphone carries speech (assumption)
    mission_h: float = 8.0


def cab_noise(c: Config):
    return data.NOISE[c.vehicle]["oct"] + c.level_shift_db


def ear_noise(c: Config):
    """Vehicle noise at the ear, under the helmet (8 bands)."""
    n = cab_noise(c).copy()
    if c.helmet:
        n = n - data.HELMET_ATT
    n = n - c.passive_extra_db
    if c.anr_bb:
        n = n - data.ANR_BB
    if c.anr_tonal_db > 0.0:
        t = tonal_share(c.vehicle)
        n = n + 10.0 * np.log10((1.0 - t) + t * 10.0 ** (-c.anr_tonal_db / 10.0))
    return n


def mic_levels(c: Config):
    """Speech and noise at the microphone output, referred to the speech level (125..8k)."""
    s = data.SPEECH_1M[c.effort] + c.mic_near_gain_db
    nz = cab_noise(c)[1:] - c.mic_rej
    return s, nz


def evaluate(c: Config, speech_ear_dba):
    """STI and constraints for a given A-weighted speech level at the ear."""
    s_mic, n_mic = mic_levels(c)
    lo, hi = CODEC_BANDS[c.codec]
    bp = band_pass_db(lo, hi)[1:]
    s_tx = s_mic + bp
    n_tx = n_mic + bp
    # volume: scale so that the speech at the ear has the requested A level
    s8 = np.concatenate([[-100.0], s_tx])
    g = speech_ear_dba - a_level(s8)
    s_ear = s_tx + g
    n_txe = n_tx + g
    n_ac = ear_noise(c)[1:]
    n_tot = db_sum(n_ac, n_txe)
    value, mti = sti.sti_indirect(s_ear, n_tot)
    noise_ear_dba = a_level(ear_noise(c))
    # earphone output while speech is on: speech plus transmitted noise
    out8 = np.concatenate([[-100.0], db_sum(s_ear, n_txe)])
    out_dba = a_level(out8)
    exposure = 10.0 * np.log10(10.0 ** (noise_ear_dba / 10.0) + c.speech_duty * 10.0 ** (out_dba / 10.0))
    twa = exposure + 10.0 * np.log10(c.mission_h / 8.0)
    peak = lin_level(np.concatenate([[-100.0], s_ear])) + data.SPEECH_CREST_DB
    return {
        "sti": value,
        "mti": mti,
        "speech_ear_dba": speech_ear_dba,
        "noise_ear_dba": noise_ear_dba,
        "out_dba": out_dba,
        "twa_dba": twa,
        "peak_db": peak,
        "tx_snr": s_mic - n_mic,
        "snr_ear": s_ear - n_tot,
        "above_ambient": speech_ear_dba - noise_ear_dba,
    }


def sweep_level(c: Config, levels=np.arange(60.0, 116.0, 1.0)):
    return [evaluate(c, L) for L in levels]


def best_operating_point(c: Config, target=data.STI_THRESHOLD):
    """The lowest speech level at the ear that reaches the target STI, and whether the
    hearing constraints hold there.  If the target is not reachable within the peak
    limit, return the level of maximum STI inside the limits."""
    rows = sweep_level(c)
    ok_peak = [r for r in rows if r["peak_db"] <= data.PEAK_AT_EAR_DB]
    hit = [r for r in ok_peak if r["sti"] >= target]
    if hit:
        r = hit[0]
        reached = True
    else:
        r = max(ok_peak, key=lambda x: x["sti"])
        reached = False
    r = dict(r)
    r["reached"] = reached
    r["twa_ok"] = r["twa_dba"] < data.TWA_LIMIT_DBA
    r["sti_max"] = max(x["sti"] for x in ok_peak)
    return r


def max_vehicle_level(c: Config, target=data.STI_THRESHOLD, shifts=np.arange(-25.0, 20.5, 1.0)):
    """Highest vehicle level (dB(A) at the crew position) at which the configuration still
    reaches the target STI with TWA < 85 dB(A) and the peak limit.  Returns (dBA, point)."""
    best = None
    for sh in shifts:
        cc = replace(c, level_shift_db=float(sh))
        r = best_operating_point(cc, target)
        if r["reached"] and r["twa_ok"]:
            best = (a_level(cab_noise(cc)), r)
    return best
