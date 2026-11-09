"""Study S7 - 28 V input stage against MIL-STD-1275E [R27].

Candidate input chain: reverse-polarity ideal diode -> TVS (spikes) -> linear
surge stopper (series MOSFET clamping the output at 33 V) -> bulk capacitor ->
wide-input DC-DC (9-36 V) -> crew-box load (constant power).

Checks, from the 1275E envelopes (V):
  surge 100 V / 50 ms, decaying to 33 V at 500 ms, 0.5 ohm source (Table I, Fig. 8)
  spike +/-250 V / 70 us, max 2 J (Fig. 7, 5.1.3.1.1)
  initial engagement surge 12 V for 1 s; cranking 16 V for 30 s (5.1.2)
Outputs: docs/img/s7_power.png, results/s7_power.json
"""

from __future__ import annotations

import numpy as np

from common import plt, save, dump  # noqa: F401

P_LOAD = 6.0  # W, crew box with headset (assumption A)
V_CLAMP = 33.0  # surge-stopper output clamp, top of the 1275E steady-state range
R_SRC = 0.5
C_BULK = 470e-6
DCDC_VMIN = 9.0  # wide-input converter minimum input (A)
DT = 1e-5
I_LIMIT = 1.0  # A, surge-stopper current limit (A)


def surge_envelope(t):
    """Upper surge envelope of Fig. 8: 100 V for 50 ms, then down to 33 V at 500 ms
    (linear between, conservative), 28 V nominal outside."""
    v = np.full_like(t, 28.0)
    on = (t >= 0.01) & (t < 0.06)
    v[on] = 100.0
    dec = (t >= 0.06) & (t < 0.51)
    v[dec] = 100.0 - (100.0 - 33.0) * (t[dec] - 0.06) / 0.45
    return v


def simulate():
    t = np.arange(0.0, 0.7, DT)
    vs = surge_envelope(t)
    vc = 28.0
    vout, pq, iin = [], [], []
    for v in vs:
        i_load = P_LOAD / max(vc, 1.0)
        # stopper: passes the input while below the clamp, else regulates the output at V_CLAMP
        v_after_src = v - R_SRC * i_load
        target = min(v_after_src, V_CLAMP)
        i_ch = min(max((target - vc) / 0.05, 0.0), I_LIMIT)  # current-limited charging of the bulk cap
        vc += (i_ch - i_load) * DT / C_BULK
        vc = min(vc, V_CLAMP)
        i = i_load + i_ch
        vout.append(vc)
        pq.append(max(v_after_src - vc, 0.0) * i)
        iin.append(i)
    pq = np.array(pq)
    return t, vs, np.array(vout), pq, np.cumsum(pq) * DT


def main():
    t, vs, vo, pq, eq = simulate()
    res = {
        "p_load_w": P_LOAD, "v_clamp": V_CLAMP,
        "mosfet_peak_w": float(pq.max()), "mosfet_energy_j": float(eq[-1]),
        "vout_max": float(vo.max()),
        # TVS must stand off the 100 V surge (else it would conduct (100 - Vc)/0.5 ohm for 50 ms)
        "tvs_if_clamped_at_36v_energy_j": float((100 - 36) / R_SRC * 36 * 0.05),
        "spike_energy_max_j": 2.0,
        # hold-up needed without a wide-input converter, from 28 V down to 16 V
        "holdup_c_for_IES_1s_f": 2 * P_LOAD * 1.0 / (28.0 ** 2 - 16.0 ** 2),
        "holdup_c_for_50ms_dropout_f": 2 * P_LOAD * 0.05 / (28.0 ** 2 - DCDC_VMIN ** 2),
        "ies_ok_with_wide_input": 12.0 > DCDC_VMIN, "cranking_ok": 16.0 > DCDC_VMIN,
    }
    fig, axs = plt.subplots(1, 2, figsize=(9, 3.2))
    axs[0].plot(t * 1e3, vs, label="1275E surge envelope (source)")
    axs[0].plot(t * 1e3, vo, label="after surge stopper")
    axs[0].axhline(33, color="k", ls=":", lw=0.8)
    axs[0].set_xlabel("time (ms)")
    axs[0].set_ylabel("V")
    axs[0].legend(fontsize=7)
    axs[0].set_title("100 V / 50 ms surge, 0.5 ohm source", fontsize=9)
    axs[1].plot(t * 1e3, pq, label="stopper MOSFET power (W)")
    ax2 = axs[1].twinx()
    ax2.plot(t * 1e3, eq, color="#d1495b", label="energy (J)")
    ax2.set_ylabel("J")
    axs[1].set_xlabel("time (ms)")
    axs[1].set_ylabel("W")
    axs[1].set_title("MOSFET stress: %.1f W peak, %.2f J" % (res["mosfet_peak_w"], res["mosfet_energy_j"]),
                     fontsize=9)
    res["fig"] = save(fig, "s7_power.png")
    dump("s7_power.json", res)
    print({k: (round(v, 4) if isinstance(v, float) else v) for k, v in res.items()})


if __name__ == "__main__":
    main()
