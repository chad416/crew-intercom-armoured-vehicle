# Progress notes (resume from here after an interruption)

Status 9 Nov 2026, about 22:15: **Rev A complete, local git only (not pushed).**

## Done

- [x] Research notes, outside git, in `work/`:
  - `wasson/chNN.txt` (printed page = PDF page − 30);
  - `wasson_notes_A.md` (ch 4–7, 13, 19–22) and `wasson_notes_B.md` (ch 26–28, 31–34);
  - `sources_research.md`: the original with vendor names. The repo copy, `docs/sources.md`, is sanitised.
- [x] `model/`: icsim (sti, chain, anr, budget, rma, data), studies S1–S7, `run_all.py`, 16 pytest tests.
- [x] `fw/`:
  - lib/icom with 1284 host checks, 100 % lines/branches/MC/DC, MISRA with D1, 34/34 traced;
  - Zephyr app;
  - Renode IT01–IT08, 8/8.
- [x] DOSSIER.md and DOSSIER.pdf (`tools/make_pdf.py`, PyMuPDF; headless Chrome hung under the tool
  shell).
- [x] README, CHANGELOG, CI config. CI has never run: no remote.

## Open

- Publish on GitHub: needs the user's OK.
- CV and email draft for the target company: each needs the user's go (kept outside this repo).
- The CCU is not built. Possible next steps:
  - analog sidetone through the codec bypass;
  - a bypass self-test in MAINT (FMEA rows 8–9);
  - measure the line width of real track tones (DOSSIER §15).

## Gotchas

- **Bash tool:** it turns `\\n` inside heredoc Python strings into real newlines. Use Edit for strings
  with `\n`.
- **Renode Robot:** `python "..."` breaks on double quotes inside; use single quotes. Compare numbers with
  `float($x)`.
- **FxLMS:** the step size μ must be scaled with the sample rate.
- **Converter delays:** TI SLAA557 is saved in `work/dl_wm8960/slaa557.pdf`. ADC A 17/fs, DAC A 21/fs.
