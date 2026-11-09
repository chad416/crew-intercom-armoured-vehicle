# Changelog

## 0.2.0 (10 Nov 2026), Rev B

- **CCU core** (`ccu.c`, CCU-01..08): supervision, mix-minus intercom, commander radio priority,
  REBROADCAST.
- **Emergency-path self-test** in MAINT (`bypass.c`, CB-50..53). It closes the latent-failure rows of the
  FMEA. Renode IT09/IT10.
- **PTT in the uplink** (CB-54).
- **Host system test:** 4 crew boxes and the CCU on encoded frames. Results:
  - crew-to-crew delay of 1 frame;
  - lost-box detection;
  - all boxes in BYPASS 20 ms after the last CCU frame;
  - commander radio takeover.
- **System-level verification matrix** (`docs/sys_traceability.md`) and its CI check (`tools/trace_sys.py`).
- **Dossier:** decision log D1–D11, revision history, updated evidence. New `OPEN_ITEMS.md` for the
  hardware-only items.
- **Evidence totals:**
  - 1370 host checks;
  - 100 % lines, branches and MC/DC over 663 lines;
  - MISRA clean apart from D1;
  - 47/47 software and 14/14 system requirements traced;
  - Renode 10/10.
- **Names:** product and vendor names removed from the whole git history.

## 0.1.0 (9 Nov 2026)

First version, local only.

- DOSSIER.md Rev A on the Wasson spine; source register in `docs/sources.md`.
- Models and studies S1–S7 (speech chain and STI, ANR, STI validation, latency, AoA, RMA, 28 V input).
- Crew-box library `fw/lib/icom`:
  - 1284 host checks;
  - 100 % lines, branches and MC/DC;
  - MISRA clean apart from D1;
  - 34/34 requirements traced.
- Zephyr app for the RT1064 and Renode tests IT01–IT08 (8/8).
- Defects found by testing and fixed:
  - double count of lost frames (IT04);
  - ANR guard reference of 0 at the first frame (host);
  - ANR step size not scaled to the sample rate (S3).
