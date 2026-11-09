# Changelog

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
