"""Checks of the model against its sources and against independent methods."""

import numpy as np
import pytest
from icsim import anr, budget, chain, data, rma, sti


# ---- STI (IEC 60268-16 ed. 5 constants) --------------------------------------------------------
def test_weights_sum_to_one():
    assert sti.ALPHA.sum() - sti.BETA.sum() == pytest.approx(1.0)


def test_clean_and_snr_points():
    lev = np.full(7, 70.0)
    assert sti.sti_indirect(lev, lev - 200, masking=False)[0] == pytest.approx(1.0)
    assert sti.sti_indirect(lev, lev, masking=False)[0] == pytest.approx(0.5)
    assert sti.sti_indirect(lev, lev + 15, masking=False)[0] == pytest.approx(0.0, abs=1e-6)


def test_masking_segments_are_continuous():
    for knee in (63.0, 67.0):
        assert sti.masking_db(knee - 1e-9) == pytest.approx(sti.masking_db(knee), abs=1e-6)
    assert sti.masking_db(100.0) == pytest.approx(-10.0)
    assert sti.masking_db(99.999) == pytest.approx(-9.8, abs=0.01)  # 0.5*L - 59.8 meets -10 within 0.2 dB


def test_rating_bands():
    assert [sti.rating(x) for x in (0.2, 0.4, 0.5, 0.7, 0.8)] == ["bad", "poor", "fair", "good", "excellent"]


def test_direct_matches_indirect():
    rng = np.random.default_rng(3)
    fs = 32000
    lev = np.array([70, 72, 70, 66, 60, 55, 50.0])
    x = sti.stipa_signal(lev, fs, 12.0, rng)
    nb = sti.octave_split(rng.standard_normal(len(x)), fs)
    n = sum(nb[k] * (20e-6 * 10 ** ((lev[k] - 3) / 20) / np.sqrt(np.mean(nb[k] ** 2))) for k in range(7))
    d = sti.sti_direct(x + n, fs)[0]
    i = sti.sti_indirect(lev, lev - 3)[0]
    assert abs(d - i) < 0.04


# ---- data checks against the published overall levels ----------------------------------------------
@pytest.mark.parametrize("veh,dba,lin", [("tracked", 111.0, 125.0), ("wheeled", 110.0, 115.0)])
def test_spectra_reproduce_published_levels(veh, dba, lin):
    oct_ = data.NOISE[veh]["oct"]
    assert chain.a_level(oct_) == pytest.approx(dba, abs=0.5)  # [R2] Fig. 2 legend
    assert chain.lin_level(oct_) == pytest.approx(
        lin, abs=2.5
    )  # 63 Hz-8 kHz only; legend covers 50 Hz-10 kHz


def test_speech_spectrum_overall():
    # ANSI S3.5 normal effort overall 62.35 dB [R14]; the octave levels are rounded to 0.1 dB and
    # use a 0.707 fc bandwidth, so the sum is 0.23 dB high
    assert 10 * np.log10(np.sum(10 ** (data.SPEECH_1M["normal"][1:] / 10))) == pytest.approx(62.35, abs=0.3)


def test_codec_band_pass():
    wb = chain.band_pass_db(50, 7000)
    assert wb[1:6] == pytest.approx(0.0)
    assert -6 < wb[7] < -4  # 8 kHz octave only partly inside 7 kHz
    nb = chain.band_pass_db(300, 3400)
    assert nb[1] < -90  # no 125 Hz band


def test_anr_lowers_ear_noise_tracked():
    c = chain.Config("x")
    assert chain.a_level(chain.ear_noise(c)) == pytest.approx(94.8, abs=0.2)
    c2 = chain.Config("x", anr_bb=True)
    assert chain.a_level(chain.ear_noise(c2)) < chain.a_level(chain.ear_noise(c)) - 8


def test_transmit_side_caps_sti():
    c = chain.Config("x", anr_bb=True, anr_tonal_db=20)
    r = chain.best_operating_point(c)
    assert r["reached"] and r["sti_max"] < data.STI_OBJECTIVE
    c5 = chain.Config("x", anr_bb=True, anr_tonal_db=20, mic_rej=data.MIC_REJ + 3)
    assert chain.best_operating_point(c5)["sti_max"] >= data.STI_OBJECTIVE


# ---- ANR -----------------------------------------------------------------------------------------------
def _run(phase_err):
    fs = 4000
    d, f0, _ = anr.track_noise(
        fs, 3.0, 40.0, data.TRACK_PITCH_M, [100.0, 96.0, 92.0], rng=np.random.default_rng(1)
    )
    r = anr.harmonic_fxlms(d, f0, fs, anr.SecondaryPath(fs, 0.003), 3, mu=0.01, s_hat_phase_err_deg=phase_err)
    if r["diverged"]:
        return None
    tail = slice(len(d) - fs // 2, len(d))
    return 10 * np.log10(np.mean(d[tail] ** 2) / np.mean(r["e"][tail] ** 2))


def test_fxlms_converges_within_90_deg():
    assert _run(0.0) > 40
    assert _run(60.0) > 20


def test_fxlms_diverges_beyond_90_deg():
    assert _run(120.0) is None


def test_broadband_bound_falls_with_delay():
    f = np.linspace(1, 8000, 2000)
    psd = 1.0 / (1 + (f / 200.0) ** 2)
    a = [anr.predictable_attenuation_db(f, psd, 16000, d) for d in (1e-4, 1e-3, 4e-3)]
    assert a[0] > a[1] > a[2] >= 0


# ---- budgets, RMA ------------------------------------------------------------------------------------------
def test_latency_budgets_inside_limits():
    assert budget.total(budget.sidetone_path()) == pytest.approx(5.375)
    assert budget.total(budget.intercom_path("A2")) < budget.LIMITS_MS["intercom"][0]


def test_series_failure_rates_add():
    assert rma.lam({"relay_gp": 2}) == pytest.approx(2.0e-6)
    assert rma.mission_reliability("A2", 12.0, 1.0, True) > rma.mission_reliability("A2", 12.0, 1.0, False)
