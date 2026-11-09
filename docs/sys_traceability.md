# System requirement verification matrix

The system requirements are listed in [DOSSIER.md §5](../DOSSIER.md). `tools/trace_sys.py` checks three
things:
- each SYS requirement has a row in this table;
- every file listed exists;
- every test listed exists.

CI runs this check.

**Methods** (Wasson ch. 13): A = analysis, T = test, I = inspection, D = demonstration.

**Status:**
- **met:** the evidence shows compliance.
- **conditional:** met only under the stated condition.
- **open (hardware):** needs hardware or a laboratory. See [OPEN_ITEMS.md](../OPEN_ITEMS.md).

| SYS | Requirement | Method | Evidence | Result | Status |
|---|---|---|---|---|---|
| SYS-3.2.1 | STI ≥ 0.41 (T) / ≥ 0.48 (O) at 110 dB(A) | A | results/s1_chain.json, results/s3_stipa.json, model/tests/test_model.py | best STI 0.44 (C3), above the threshold. The objective needs +3 dB transmit SNR (C5: 0.52). Model validated against direct STIPA within 0.012 | conditional (objective: microphone) |
| SYS-3.2.2 | 8-h TWA at the ear < 85 dB(A) | A | results/s1_chain.json | 89.9 dB(A) at 111 dB(A) with the helmet and both ANRs (2.6 h allowed). 77.0 dB(A) with a +10 dB earplug (C6) | conditional (decision D3: earplug) |
| SYS-3.2.3 | ≤ 115 dB peak at the ear | A, T | results/s1_chain.json, fw/tests/unit/test_mix.c, fw/tests/robot/icom.robot | operating point 104–106 dB peak; limiter at 0.5 FS (CB-34); anti-noise clamp seen holding at 16 384 in IT06 | met |
| SYS-3.2.4 | sidetone delay ≤ 10 ms | A | results/s4_budget.json | 5.4 ms worst case (margin 46 %) | met (digital); see D4 |
| SYS-3.2.5 | crew-to-crew delay ≤ 20 ms | A, T | results/s4_budget.json, fw/tests/unit/test_system.c | 8.6 ms worst-case budget; the host system test measures 1 frame of bus pipeline | met |
| SYS-3.2.6 | binaural skew ≤ 1 ms | I | fw/app/src/main.c | one codec output frame drives both earphones | met (design) |
| SYS-3.2.7 | voice resumes ≤ 50 ms after a single failure | T, A | fw/tests/robot/icom.robot, fw/tests/unit/test_system.c, results/fmea.md | BYPASS 20.3 ms after the last frame (IT02); relay drop 1.4–2.1 ms after a CPU stall (IT08); every live box in BYPASS 20 ms after the last CCU frame (system test); the emergency path is self-tested (IT09/IT10) | met in emulation; relay operate time is open (hardware) |
| SYS-3.2.8 | 12-h mission reliability ≥ 0.9999 | A | results/s6_rma.json | 37 losses per 10^6 missions (0.99996) at 90 % BIT coverage | met (prediction) |
| SYS-3.3.1 | warnings heard over every other channel | T | fw/tests/unit/test_mix.c, fw/tests/unit/test_ccu.c | alarm at rank 0; lower channels −12 dB (CB-30); the CCU carries the alarm to every box (CCU-03) | met |
| SYS-3.3.2 | sidetone in phase, unfiltered | I | fw/lib/icom/src/mix.c | the digital sidetone is unfiltered but 5.4 ms late | open (hardware): codec analog sidetone, D4 |
| SYS-3.3.3 | mic ≥ 10 dB better in ≥ 100 dB LF noise | A | results/s1_chain.json | the vendor pressure-gradient curve gives ≥ 10 dB only below about 800 Hz | open (hardware): headset procurement |
| SYS-3.4.1 | 20–33 V; rides through 1275E transients | A | results/s7_power.json | stopper MOSFET 6.9 J, 87 W peak; the TVS must stand off 100 V; a 9–36 V converter covers IES and cranking | met by analysis; bench test open (hardware) |
| SYS-3.5.1 | MIL-STD-461G Army ground | T | DOSSIER.md | test plan §11.1 | open (laboratory) |
| SYS-3.5.2 | MIL-STD-810H methods | T | DOSSIER.md | test plan §11.2 | open (laboratory) |
