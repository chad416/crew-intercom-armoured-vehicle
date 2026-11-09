# Progress notes (resume from here after an interruption)

Order: 1) dossier (DOSSIER.md + PDF), 2) model/prototype answering the question, tests, CI.

- [x] work/wasson/chNN.txt extracted (printed page = PDF page - 30)
- [x] work/wasson_notes_A.md (ch 4-7, 13, 19-22)
- [x] work/wasson_notes_B.md (ch 26-28, 31-34)
- [ ] work/sources_research.md (noise, STI, headsets, HF limits, MIL stds) - agent
- [ ] model/ Python: noise, STI (IEC 60268-16 indirect + STIPA direct), headset+ANR, FxLMS, ENC, latency budget, AoA, RMA, 1275E power
- [x] fw/lib/icom C lib + host tests: 1282 checks, 100 % lines/branches/MC/DC, MISRA clean (D1 = 2.5 on public header)
- [ ] fw/app Zephyr app + fw/sim Renode + fw/tests/robot
- [ ] DOSSIER.md + DOSSIER.pdf
- [ ] CI, README, commit; update the outreach plan (outside this repo); memory

Notes: codec group delay source = TI SLAA557 (TLV320AIC3204 app ref guide) Tables 2-11..2-13, 2-19..2-21: ADC A 17/fs, B 11/fs, C 11/fs; DAC A 21/fs, B 18/fs, C 13/fs. Saved in work/dl_wm8960/slaa557.pdf.
Bash tool turns \n inside heredoc python strings into real newlines: use Edit for C strings with 
.
