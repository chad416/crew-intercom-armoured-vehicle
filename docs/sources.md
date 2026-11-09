# Sources: what every number rests on

This is the source register for DOSSIER.md. It was compiled by web research on 9 Nov 2026 and lightly edited: vendor and vehicle product names are replaced by generic descriptions, under the repository's naming rule. The reference numbers here are this file's own; DOSSIER.md uses [Rn] keys that map to the same documents.

Compiled 9 Nov 2026 (web research). Every number is tied to a source.

**Status labels**
- **VERIFIED**: I read the number in the source text or table myself (PDF text extraction, or a rendered page image).
- **VERIFIED (graph)**: read off a published graph from a rendered page image. The reading uncertainty is stated (typically +/-1 to 2 dB).
- **DERIVED**: computed by me from VERIFIED values. The method is given, so it can be re-done.
- **UNVERIFIED**: from a secondary summary, a search snippet or memory. I did not see it in the source. Do not use it without checking.

**Access notes**
- DTIC (apps.dtic.mil) showed an "Under Maintenance" page throughout this session, so no DTIC PDFs could be read directly. The NATO STO copies of the same papers were used instead.
- MIL-STD-1472H (12 MB) was too large to fetch. MIL-STD-1472G w/Change 1 (2019) was read in its place.
- MIL-STD-810H in full was not fetched. Method 501.7 and Method 502.7 excerpts (DLA ASSIST copies hosted by a vendor) were read.
- Newer revisions exist and contracts may cite them. The search results showed MIL-STD-461H (dated 17 Apr 2026, per an a test-lab PDF) and MIL-STD-1275F (2022, per standards resellers). Both are UNVERIFIED and were not read.
- Vendor and vehicle product names are withheld in this copy.

---

## 1. Interior noise of armoured vehicles

### 1a. Tracked APC (tracked APC A / B), overall A-weighted levels: NATO RTO-MP-AVT-110 paper 9 [1]

Test speeds were idle, 20 km/h, 40 km/h and max (about 60 km/h), on pavement and gravel. There were 3 microphones: driver, crew commander, and crew compartment (microphone #3).

| quantity | value | unit | source (full citation) | where in source | URL | status |
|---|---|---|---|---|---|---|
| tracked APC A driver, pavement, hatches open, 20 km/h | 113.11 | dB(A) | [1] | Table IIa, p. 9-10 | https://publications.sto.nato.int/publications/STO%20Meeting%20Proceedings/RTO-MP-AVT-110/MP-AVT-110-09.pdf | VERIFIED |
| tracked APC A driver, pavement, hatches open, 40 km/h | 115.63 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A driver, pavement, hatches open, max speed | 116.51 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A driver, pavement, hatches closed, 20 / 40 / max | 112.95 / 115.56 / 115.61 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A crew commander, pavement, hatches closed, 20 / 40 / max | 111.67 / 116.67 / 116.36 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A crew compartment, pavement, hatches closed, 20 / 40 / max | 109.72 / 114.91 / 112.29 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A crew compartment, pavement, hatches open, 40 km/h | 113.9 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC B (lengthened variant) crew compartment, pavement, hatches closed, fan normal, 40 km/h / max | 117.09 / 118.21 | dB(A) | [1] | Table IIa | same | VERIFIED |
| tracked APC A idle, all positions, pavement | 83.4 to 87.6 | dB(A) | [1] | Table IIa | same | VERIFIED |
| Track-impact tonal components | "on the order of 120" | dB | [1] | Sec. 5 text, p. 9-6/9-7 | same | VERIFIED |
| Track fundamental frequency at 20 / 40 km/h (track pitch 0.149 m) | 37.3 / 74.6 | Hz | [1] | Sec. 5 text | same | VERIFIED |
| Track fundamental frequency at max speed | about 105 to 120 | Hz | [1] | Sec. 5 text | same | VERIFIED |
| tracked APC A fan blade-passing range (engine 600 to 2800 rpm) | 240 to 1120 | Hz | [1] | Sec. 5 text | same | VERIFIED |

Caveat: in [1], the captions of Tables IIb and IIIa ("L-weighted on gravel", "A-weighted on gravel") look swapped or mislabelled. Their values (about 125 dB for tracked APC A at 40 km/h in "IIIa") look like linear levels. Only Table IIa (pavement, A-weighted) is used here.

### 1b. Tracked APC, 1/3-octave spectrum: tracked APC A, crew compartment (mic #3), pavement, hatches open, fan max, 40 km/h [1, Fig. 6, p. 9-15]

Read from the bar chart, +/-2 dB. The bars appear to be **A-weighted**. Check: their energy sum (125 Hz to 5 kHz) is 113.6 dB, which matches the 113.9 dB(A) in Table IIa. The figure's own "A" bar is also about 114 dB. This weighting is an inference, so treat it as DERIVED.

| 1/3-oct centre (Hz) | 50 | 63 | 80 | 100 | 125 | 160 | 200 | 250 | 315 | 400 | 500 | 630 | 800 | 1k | 1.25k | 1.6k | 2k | 2.5k | 3.15k | 4k | 5k |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| level, dB (A-wtd, inferred) | 74 | 85 | 101 | 84 | 91 | 102 | 97 | 101 | 103 | 102 | 103 | 102 | 101 | 102 | 102 | 101 | 100 | 99 | 99 | 99 | 99 |

Octave sums (DERIVED): 63 Hz 101.1, 125 Hz 102.4, 250 Hz 106.6, 500 Hz 107.3, 1 kHz 106.6, 2 kHz 104.9, 4 kHz 103.8 dB(A). Status: VERIFIED (graph) for the bands, DERIVED for the octave sums.

### 1c. Tracked main battle tank and wheeled "light wheeled tank" at 60 km/h, driver ("pilot") position, 1/3-octave LINEAR spectra: Buck, Hamery, Zimpfer, ICA 2010 [2]

| quantity | value | unit | source | where in source | URL | status |
|---|---|---|---|---|---|---|
| Battle tank (tracked), overall | 125 dB(Lin) / 111 dB(A) | dB | [2] | Fig. 1 and Fig. 2 legends, p. 2 | https://www.acoustics.asn.au/conference_proceedings/ICA2010/cdrom-ICA2010/papers/p495.pdf | VERIFIED |
| Light wheeled tank (wheeled), overall | 115 dB(Lin) / 110 dB(A) | dB | [2] | Fig. 1/Fig. 2 legends | same | VERIFIED |
| Armoured personnel carrier, overall | 109 dB(Lin) / 94 dB(A) | dB | [2] | Fig. 1/Fig. 2 legends (the Table 1 text shows 95 dB(A)) | same | VERIFIED |
| Light vehicle, overall | 101 dB(Lin) / 82 dB(A) | dB | [2] | Fig. 1/Fig. 2 legends | same | VERIFIED |
| Speed for these exposures | 60 | km/h | [2] | text p. 2 ("at a speed of 60 km/h") and Table 1 | same | VERIFIED |
| A vs Lin difference, 3 of 4 vehicles | 14 to 19 | dB | [2] | text p. 2 | same | VERIFIED |
| Wheeled tank: A-weighting reduction smaller because of strong 250 Hz to 2 kHz content | 5 | dB | [2] | text p. 2 | same | VERIFIED |

1/3-octave linear levels read from Fig. 1 (zoomed render, +/-1.5 dB):

| 1/3-oct (Hz) | 50 | 63 | 80 | 100 | 125 | 160 | 200 | 250 | 315 | 400 | 500 | 630 | 800 | 1k | 1.25k | 1.6k | 2k | 2.5k | 3.15k | 4k | 5k | 6.3k | 8k | 10k |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Battle tank (tracked), dB Lin | 98 | 102 | 121 | 120 | 107 | 114 | 114 | 109 | 102 | 101 | 101 | 102 | 98 | 96 | 95 | 95 | 94 | 94 | 91 | 88 | 86 | 84 | 81 | 78 |
| Light wheeled tank, dB Lin | 94 | 88 | 92 | 93 | 90 | 94 | 95 | 108 | 95 | 94 | 102 | 105 | 102 | 105 | 97 | 93 | 95 | 92 | 94 | 87 | 82 | 80 | 77 | 72 |

Octave-band levels, DERIVED by energy-summing three 1/3-octaves from the table above:

| octave centre (Hz) | 63 | 125 | 250 | 500 | 1k | 2k | 4k | 8k | check: A-wtd sum |
|---|---|---|---|---|---|---|---|---|---|
| Battle tank (tracked), dB Lin | 121.2 | 121.3 | 115.6 | 106.3 | 101.5 | 99.1 | 93.7 | 86.7 | 111.3 dB(A) (paper: 111) |
| Light wheeled tank, dB Lin | 96.7 | 97.5 | 108.8 | 106.8 | 107.4 | 98.4 | 95.0 | 82.4 | 110.1 dB(A) (paper: 110) |

The A-weighted sums match the published dB(A) values within 0.3 dB, so the graph reading holds. **dB(C) is not given in [1] or [2].** It can be derived from the octave table with C-weighting, but that has not been done here.

### 1d. Survey-level data

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| A-weighted interior levels for 26 NATO land vehicles, grouped (cargo, high-mobility, APC, light tank, heavy tank) | about 80 to >110 | dB(A) | van Wijngaarden & James, RTO-MP-AVT-110 KN1 [3] | Fig. 2, p. KN1-7; discussion record p. KN1-18 ("from 80 dBA to more than 110 dBA") | https://publications.sto.nato.int/publications/STO%20Meeting%20Proceedings/RTO-MP-AVT-110/MP-AVT-110-$KN1.pdf | VERIFIED (range); per-vehicle values not read |
| APC passenger-compartment levels vs driver position | up to 8 dB lower | dB | [3] | p. KN1-7 text | same | VERIFIED |
| Heavy tanks: highest levels, small spread between types | qualitative | - | [3] | p. KN1-7 text | same | VERIFIED |
| a tracked IFV in motion | 101 to 115 | dB(A) | US Army hearing-conservation page (copy in NRC ADAMS ML070120304) | - | https://www.nrc.gov/docs/ML0701/ML070120304.pdf | UNVERIFIED (search snippet only) |
| an articulated tracked vehicle crew noise/vibration | not extracted | - | ResearchGate 346148620 | - | https://www.researchgate.net/publication/346148620 | not read |

---

## 2. MIL-STD-1474E (15 Apr 2015) and MIL-STD-1472 speech criteria

### 2a. MIL-STD-1474E [4]

| quantity | value | unit | source | where in source | URL | status |
|---|---|---|---|---|---|---|
| Limit A: steady-state SPL shall not equal or exceed | 85 | dB(A) | [4] | 4.2.3.1 a, p. 10 | https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1474E_52224/ | VERIFIED |
| Limit B: 8-h TWA shall not equal or exceed | 85 | dB(A) | [4] | 4.2.3.1 b | same | VERIFIED |
| Limit C: 8-h TWA at the ear of personnel wearing hearing protection; helmet/headset attenuation counts | 85 | dB(A) | [4] | 4.2.3.1 c, p. 10 | same | VERIFIED |
| Maximum steady-state SPL after hearing protection | 130 | dB(A) | [4] | 4.2.3.2 | same | VERIFIED |
| Whole-body exposure limit, regardless of protection | 150 | dB overall | [4] | 4.2.3.3 | same | VERIFIED |
| Speech communication: MRT score (corrected for guessing, ANSI/ASA S3.2), minimum | 80 | % | [4] | 4.3, p. 10 | same | VERIFIED |
| Predicted intelligibility for the worst nominal case must equal at least this MRT level (else MRT test required) | 85 | % MRT | [4] | 4.3.1, p. 10-11 | same | VERIFIED |
| Assumed voice for predictions | "raised" (ANSI/ASA S3.5), 30 cm mouth-to-ear | - | [4] | 4.3.1 | same | VERIFIED |
| Table I, MRT 95 % -> AI / STI / SII | 0.59 / 0.57 / 0.66 | - | [4] | Table I, p. 11 | same | VERIFIED |
| Table I, MRT 90 % | 0.50 / 0.48 / 0.56 | - | [4] | Table I | same | VERIFIED |
| Table I, MRT 85 % | 0.42 / 0.41 / 0.49 | - | [4] | Table I | same | VERIFIED |
| Table I, MRT 80 % | 0.35 / 0.36 / 0.42 | - | [4] | Table I | same | VERIFIED |
| Table I, MRT 75 % | 0.30 / 0.32 / 0.37 | - | [4] | Table I | same | VERIFIED |
| Table I, MRT 70 / 65 / 60 / 55 / 50 % -> STI | 0.29 / 0.27 / 0.25 / 0.24 / 0.24 | - | [4] | Table I | same | VERIFIED |
| Table I, note 1: AI = 2.35 MRT^3 - 2.91 MRT^2 + 1.27 MRT (Webster & Allen 1972) | formula | - | [4] | Table I note 1 | same | VERIFIED |
| Table I, note 2: STI = 10^(-0.77 AI^2 + 1.56 AI + 0.1) - 1 (after IEC 60849-2) | formula | - | [4] | Table I note 2 (printed as "10[ ... -1]") | same | VERIFIED |
| Table I, note 3: SII = 0.9915 AI + 0.0721 (Bradley 2003) | formula | - | [4] | Table I note 3 | same | VERIFIED |
| Frequency range for communication requirements (1/3-octave centres) | 160 to 5000 | Hz | [4] | 4.3.2 | same | VERIFIED |
| MRT guessing correction (50-word list): % = 2.4 x correct - 20 | formula | - | [4] | 4.3.3.1 | same | VERIFIED |
| Emulated-noise MRT: reproduce each octave 125 Hz to 4 kHz within / overall within | 3 / 1 | dB | [4] | 4.3.3.4 | same | VERIFIED |
| Table A-I Cat. A: no direct person-to-person voice communication required | SPL >= 100 | dB(A) | [4] | App. A, Table A-I, p. 32 | same | VERIFIED |
| Cat. B: electrically aided communication via attenuating helmet or headset required | SPL < 100 | dB(A) | [4] | Table A-I | same | VERIFIED |
| Cat. C: occasional shouted communication possible at 30 cm | SPL < 90 | dB(A) | [4] | Table A-I | same | VERIFIED |
| Cat. D: occasional shouted communication possible at 60 cm | SPL < 85 | dB(A) | [4] | Table A-I | same | VERIFIED |
| Cat. E: occasional telephone/radio use, communication up to 1.5 m (about NC-70) | SPL < 75 | dB(A) | [4] | Table A-I | same | VERIFIED |
| Cat. F: frequent telephone/radio use up to 1.5 m (about NC-60) | SPL < 65 | dB(A) | [4] | Table A-I | same | VERIFIED |
| Single item, total system configuration unknown: limit is this much below the system category | 3 | dB | [4] | A.5.1.1 | same | VERIFIED |

### 2b. MIL-STD-1472 speech intelligibility criteria

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Exceptionally high intelligibility: MRT / AI | 97 % / 0.7 | - | MIL-STD-1472G w/Change 1 (17 Jan 2019) [5] | 5.3.1.8.1 b, TABLE XX, p. 130 | https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1472G_CHG-1_56051/ | VERIFIED (1472G CHG-1) |
| Normal acceptable intelligibility: MRT / AI | 91 % / 0.5 | - | [5] | TABLE XX | same | VERIFIED |
| Minimally acceptable intelligibility for mechanized equipment users: MRT / AI | 80 % / 0.4 | - | [5] | TABLE XX | same | VERIFIED |
| Minimally acceptable intelligibility: MRT / AI | 75 % / 0.3 | - | [5] | TABLE XX | same | VERIFIED |
| Same four rows in MIL-STD-1472H (15 Sep 2020) | assumed unchanged; table number unknown | - | MIL-STD-1472H [6] | not read (file too large) | https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1472H_57041/ | UNVERIFIED |
| PB-word score columns | not present in 1472G CHG-1 Table XX (MRT and AI only) | - | [5] | TABLE XX | same | VERIFIED (absence) |

---

## 3. STI rating bands

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| bad | 0 to 0.30 | STI | NTi Audio know-how page, citing IEC 60268-16 [7]; also the Wikipedia table citing IEC 60268-16 ed.4 2011 [8] | rating table | https://www.nti-audio.com/en/support/know-how/how-do-we-measure-speech-intelligibility-sti | VERIFIED (secondary) |
| poor | 0.30 to 0.45 | STI | [7], [8] | same | same | VERIFIED (secondary) |
| fair | 0.45 to 0.60 | STI | [7], [8] | same | same | VERIFIED (secondary) |
| good | 0.60 to 0.75 | STI | [7], [8] | same | same | VERIFIED (secondary) |
| excellent | 0.75 to 1.00 | STI | [7], [8] | same | same | VERIFIED (secondary) |
| Approximate intelligibility for the bands (syllables / words / sentences): bad 0-34 / 0-67 / 0-89 %, poor 34-48 / 67-78 / 89-92 %, fair 48-67 / 78-87 / 92-95 %, good 67-90 / 87-94 / 95-96 %, excellent 90-96 / 94-96 / 96-100 % | % | - | [8] | "Scale" table | https://en.wikipedia.org/wiki/Speech_transmission_index | VERIFIED (secondary; the primary is ISO 9921 Annex F, not read) |
| ISO 9921:2003 has an "Annex F (informative) Intelligibility ratings for speech communications" | title only | - | ISO 9921:2003 [9] | ToC (via search result) | https://www.iso.org/standard/33589.html | UNVERIFIED (contents not read) |
| IEC 60268-16:2020 Annex F "Nominal qualification bands for STI" (Fig. F.1) and Annex G "Examples of STI qualification bands and typical applications" (Table G.1) exist | titles | - | IEC 60268-16:2020 preview [10] | ToC, pp. 70-71 | https://cdn.standards.iteh.ai/samples/21925/7a5eb0d5e54e49e685b89ede43782760/IEC-60268-16-2020.pdf | VERIFIED (titles only) |
| Qualification bands A+ .. U (nominal STI): A+ >0.76; A 0.74; B 0.70; C 0.66; D 0.62; E 0.58; F 0.54; G 0.50; H 0.46; I 0.42; J 0.38; U <0.36, each band +/-0.02 around the nominal | from memory | STI | IEC 60268-16 ed.4/ed.5 Annex F | Fig. F.1 | - | **UNVERIFIED (memory; not found in any open source this session)** |

---

## 4. IEC 60268-16 indirect method: constants and equations

Primary source: IEC 60268-16:2020 ed.5 (paywalled). The preview [10] confirms the table numbering. Table A.1 is "MTI octave band weighting factors" (p. 48), A.2 is "Auditory masking as a function of the octave band level" (p. 53), A.3 is "Absolute speech reception threshold level in octave bands" (p. 54), A.4 is "Octave band levels relative to the A-weighted speech level" (p. 55), and B.1 is "Modulation frequencies for the STIPA method" (p. 57). The values come from open reproductions: the AIJ benchmark pages [11] (which state compliance with ed.5), MathWorks [12], and Rajmic et al. [13].

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| alpha_k (male), 125 / 250 / 500 / 1k / 2k / 4k / 8k Hz | 0.085 / 0.127 / 0.230 / 0.233 / 0.309 / 0.224 / 0.173 | - | [11], [12] (reproducing IEC 60268-16:2020 Table A.1) | AIJ B-32 / A-21 "Table A.1"; MathWorks "A.2.2" | https://news-sv.aij.or.jp/kankyo/s24/benchmark/b32/b32.html | VERIFIED (secondary) |
| beta_k (male), between 125-250 / 250-500 / 500-1k / 1k-2k / 2k-4k / 4k-8k | 0.085 / 0.078 / 0.065 / 0.011 / 0.047 / 0.095 | - | [11], [12] | Table A.1 | same | VERIFIED (secondary) |
| Female factors | removed in ed.5 | - | [10] | Foreword, technical changes item d) | iTeh preview | VERIFIED |
| Auditory masking L_a,k (dB), set by the level L_(k-1) of the octave below: L_(k-1) < 63 dB | 0.5 L_(k-1) - 65.0 | dB | [11] (Table A.2) | AIJ B-32 and A-21 | https://news-sv.aij.or.jp/kankyo/s24/benchmark/a21/a21.html | VERIFIED (secondary) |
| 63 <= L_(k-1) < 67 dB | 1.8 L_(k-1) - 146.9 | dB | [11] | Table A.2 | same | VERIFIED (secondary) |
| 67 <= L_(k-1) < 100 dB | 0.5 L_(k-1) - 59.8 | dB | [11] | Table A.2 | same | VERIFIED (secondary) |
| L_(k-1) >= 100 dB | -10 | dB | [11] | Table A.2 | same | VERIFIED (secondary) |
| Masking intensity: I_am,k = I_(k-1) x 10^(L_a,k/10); none applied to band 1 (125 Hz) | formula | - | [13] Step 6; [11] Steps 9-11 | - | https://arxiv.org/html/2510.03825v1 | VERIFIED (secondary) |
| Absolute speech reception threshold A_k, 125 / 250 / 500 / 1k / 2k / 4k / 8k | 46 / 27 / 12 / 6.5 / 7.5 / 8 / 12 | dB SPL | [11] (Table A.3) | AIJ B-32 and A-21 | as above | VERIFIED (secondary) |
| Male test-signal octave levels relative to the A-weighted speech level, 125 .. 8k | -2.5 / +0.5 / 0 / -6 / -12 / -18 / -24 | dB | [11] (Table A.4); [12] | - | as above | VERIFIED (secondary) |
| MTF with noise: m' = m x I_s / (I_s + I_n) | formula | - | [13] Step 5 | - | arXiv 2510.03825 | VERIFIED (secondary) |
| MTF with masking and threshold: m'' = m' x I_k / (I_k + I_am,k + I_rt,k), with I_rt,k = 10^(A_k/10) | formula | - | [13] Step 6 | - | same | VERIFIED (secondary) |
| Effective SNR: SNR_eff = 10 log10(m / (1 - m)), clipped to [-15, +15] | dB | - | [13] Step 7; [11] Step 15 | - | same | VERIFIED (secondary) |
| Transmission index: TI = (SNR_eff + 15) / 30 | formula | - | [13] Step 8; [12] code `SNR = refTI*30 - 15` | - | same | VERIFIED (secondary) |
| MTI_k = mean of the TI values over the modulation frequencies (n = 14 full STI, n = 2 STIPA) | formula | - | [13] Step 9 | - | same | VERIFIED (secondary) |
| STI = sum_(k=1..7) alpha_k MTI_k - sum_(k=1..6) beta_k sqrt(MTI_k MTI_(k+1)); clip to 1 | formula | - | [13] Step 10; [11] Step 18 | - | same | VERIFIED (secondary). Note: the MathWorks page text drops the sqrt. Use the sqrt form (AIJ and Rajmic agree). |
| 14 modulation frequencies: f_m = 10^((m-2)/10), m = 0..13, nominal 0.63, 0.8, 1, 1.25, 1.6, 2, 2.5, 3.15, 4, 5, 6.3, 8, 10, 12.5 | Hz | - | [12] code `f_m = 10.^((-2:11)/10)`; nominal list from a search snippet (IoA Bjor paper) | MathWorks C.3.3 | https://au.mathworks.com/help/audio/ug/speech-transmission-index-iec60268-16-edition-5-compliance.html | VERIFIED (formula) / UNVERIFIED (rounded nominal list) |
| STIPA pairs (f1, f2), 125 Hz: 1.6, 8; 250: 1.0, 5.0; 500: 0.63, 3.15; 1k: 2.0, 10.0; 2k: 1.25, 6.25; 4k: 0.8, 4.0; 8k: 2.5, 12.5 | Hz | - | [12] `tableB1` (= IEC Table B.1) | MathWorks supporting functions | same | VERIFIED (secondary) |
| STIPA test-signal modulation depth | 0.55 | - | [13] Eq. 4 | - | arXiv | VERIFIED (secondary) |
| Theoretical MTF of a reverberant room: m = 1 / sqrt(1 + (2 pi f_m T / 13.8)^2) | formula | - | [12] (Eq. C.3) | - | MathWorks | VERIFIED (secondary) |

---

## 5. Speech spectra by vocal effort, and the Lombard effect

ANSI S3.5-1997 (R2017) Table 4 (octave-band SII procedure) gives the **standard speech spectrum level** (dB per Hz) at 1 m in front of the talker. The values below are reproduced by the R package "SII" documentation [14]. The octave-band levels in the second table are DERIVED as spectrum level + 10 log10(bandwidth), with bandwidth = 0.7071 x f_c. Check: the normal-effort octaves sum to 62.3 dB, which matches the package's overall 62.35 dB.

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Overall speech SPL: normal / raised / loud / shout | 62.35 / 68.34 / 74.85 / 82.30 | dB | [14] (ANSI S3.5-1997) | `overall.spl` | https://rdrr.io/rforge/SII/man/critical.html | VERIFIED (secondary) |
| Spectrum level, normal: 250 / 500 / 1k / 2k / 4k / 8k Hz | 34.75 / 34.27 / 25.01 / 17.32 / 9.33 / 1.13 | dB/Hz | [14] (Table 4) | `octave` | same | VERIFIED (secondary) |
| Spectrum level, raised | 38.98 / 40.15 / 33.86 / 25.32 / 16.78 / 5.07 | dB/Hz | [14] | same | same | VERIFIED (secondary) |
| Spectrum level, loud | 41.55 / 44.85 / 42.16 / 34.39 / 25.41 / 11.39 | dB/Hz | [14] | same | same | VERIFIED (secondary) |
| Spectrum level, shout | 42.50 / 49.24 / 51.31 / 44.32 / 34.41 / 20.72 | dB/Hz | [14] | same | same | VERIFIED (secondary) |
| Reference distance | 1 m (standard definition) | m | ANSI S3.5-1997 | - | - | UNVERIFIED (the package page does not state it) |
| ANSI S3.5 octave procedure: no 125 Hz band (6 bands, 250 Hz to 8 kHz) | - | - | [14] | `octave` | same | VERIFIED |

Octave-band speech level at 1 m (DERIVED from the table above; +10log10(BW) = 22.5, 25.5, 28.5, 31.5, 34.5, 37.5 dB):

| effort | 250 | 500 | 1k | 2k | 4k | 8k |
|---|---|---|---|---|---|---|
| normal | 57.2 | 59.8 | 53.5 | 48.8 | 43.8 | 38.6 |
| raised | 61.5 | 65.7 | 62.4 | 56.8 | 51.3 | 42.6 |
| loud | 64.0 | 70.4 | 70.7 | 65.9 | 59.9 | 48.9 |
| shout | 65.0 | 74.8 | 79.8 | 75.8 | 68.9 | 58.2 |

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Lombard slope (Lazarus 1986, as cited) for noise above 40 to 50 dB(A) | 0.3 to 0.6 | dB/dB | Bottalico et al. 2017 [15] citing Lazarus 1986 | Introduction | https://pmc.ncbi.nlm.nih.gov/articles/PMC5612409 | VERIFIED (secondary) |
| Lombard slope above the change point of 43.3 dB(A) | 0.65 | dB(A)/dB(A) | [15] | Results/Abstract | same | VERIFIED |
| Slope between 20 and 43.3 dB(A) | 0.24 | dB/dB | [15] | Results | same | VERIFIED |
| Pearsons, Bennett & Fidell 1977 (EPA-600/1-77-025) overall slope | about 0.6 overall, about 0.8 in 1-3 kHz (as quoted in a patent) | dB/dB | Pearsons et al. 1977 [16] | - | https://www.nonoise.org/epa/Roll2/roll2doc16.pdf | UNVERIFIED (not read) |

---

## 6. Passive attenuation, ANR, and the noise-cancelling microphone

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| crew helmet (MIL-DTL-44117B) real-ear attenuation: 125 / 250 / 500 / 1k / 2k / 3.15k / 4k / 6.3k / 8k Hz | 14 / 16 / 21 / 23 / 28 / 35 / 35 / 35 / 30 | dB | MIL-DTL-44117B, Helmet, Combat Vehicle Crewman, Ballistic Shell [17] | Table I "Sound attenuation", p. 13 of the eMilSpec reproduction. Not labelled as minimum, no SD column | https://armorpersonal.emilspec.com/MIL-DTL-44117/page13.html | VERIFIED (secondary reproduction of the spec) |
| Aviation helmet, ISO 4869-1 REAT, mean: 125 / 250 / 500 / 1k / 2k / 4k / 8k Hz | 18 / 15 / 20 / 29 / 37 / 47 / 47 | dB | van Wijngaarden & James [3] (an aviation helmet) | Fig. 6, p. KN1-12 | MP-AVT-110-$KN1.pdf | VERIFIED (graph, +/-2 dB) |
| Circumaural headset, ISO 4869-1 REAT, mean: same bands | 20 / 28 / 28 / 32 / 29 / 35 / 35 | dB | [3] (a circumaural headset) | Fig. 6 | same | VERIFIED (graph, +/-2 dB) |
| ANR extra attenuation, prototype ANR headset (TNO 1997), mic at the ear-canal entrance, 1/3-oct: 50 / 63 / 80 / 100 / 125 / 160 / 200 / 250 / 315 / 400 / 500 Hz | 16 / 16.5 / 17 / 16 / 16 / 14 / 13 / 13 / 11 / 8 / 2.5 | dB | [3] | Fig. 7, p. KN1-12 ("ANR effect") | same | VERIFIED (graph, +/-2 dB) |
| ANR effect, 630 / 800 / 1k / 1.25k / 1.6k / 2k Hz (negative = amplification) | -3 / -5 / -4 / -3 / -1.5 / -1 | dB | [3] | Fig. 7; text: "around 1000-2000 Hz (where ANR actually amplifies the sound somewhat)" | same | VERIFIED (graph) |
| ANR effect above 2.5 kHz | about 0 | dB | [3] | Fig. 7 | same | VERIFIED (graph) |
| ANR noise headset (ANR on), total: 63 Hz about 21, 250 Hz about 27, 1 kHz about 18, 4 kHz about 38 | dB | - | [3] | Fig. 7 | same | VERIFIED (graph, +/-2 dB) |
| ANR instability: unwanted sounds at band edges, low-frequency noise 5 to 50 Hz in fluctuating noise | qualitative | - | [3] | p. KN1-12/13 | same | VERIFIED |
| Noise-cancelling microphone: required improvement in peak-speech-to-rms-noise ratio vs a non-noise-cancelling mic, in noise >= 100 dB overall, low-frequency | >= 10 | dB | MIL-STD-1472G CHG-1 [5] | 5.3.1.6 d(2), p. 126 | everyspec 1472G CHG-1 | VERIFIED |
| High-pass filter in predominantly low-frequency noise | 300 | Hz | [5] | 5.3.1.6 d | same | VERIFIED |
| Noise shield (talker in intense noise): minimum volume / vent area | 250 cm3 / 65 mm2 | - | [5] | 5.3.1.6 g | same | VERIFIED |
| Pressure-gradient boom mic (vendor datasheet): far-field crossover / far-field response slope | 2500 Hz / -6 dB/octave | - | vendor boom-microphone datasheet [18] (name withheld) | spec list | (URL withheld: vendor) | VERIFIED (vendor) |
| Noise-cancelling mic achievable reduction, late 1990s elements | 10 to 20 dB, only below about 3 kHz | dB | US Patent 6,009,184 [19] | background section | https://patents.google.com/patent/US6009184 | UNVERIFIED (search snippet; patent text not opened) |
| Double protection (earplug + earmuff): combined attenuation < sum of the two | qualitative | - | [3] | Fig. 8, p. KN1-13 | same | VERIFIED |
| Headset attenuation must bring ambient below this when ambient >= 85 dB(A) | < 85 | dB(A) | [5] | 5.3.1.6.1 d(2) and 5.3.1.8.2 b(2) | same | VERIFIED |
| ANSI S12.6 REAT per-band data for a generic circumaural headset | - | - | - | - | - | MISSING (no open table found; use [3] Fig. 6) |

---

## 7. Latency and sidetone (human factors)

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Own-voice delay (bone/air path + aided path) rated "disturbing" for mild to moderate loss (simulated) | 20 to 30 | ms | Stone & Moore 1999, Ear Hear 20(3):182-192 [20] | Abstract, RESULTS | PMID 10386846 | VERIFIED |
| 1999 conclusion: likely to become disturbing for delays exceeding | 20 | ms | [20] | Abstract, CONCLUSIONS | same | VERIFIED |
| Moderately severe loss: "disturbing" not quite reached at | 40 | ms | [20] | Abstract | same | VERIFIED |
| Speaking normal-hearing subjects: delay rated "disturbing" | 25 to 30 | ms | Stone & Moore 2002, Ear Hear 23(4):325-338 [21] | Abstract, RESULTS | PMID 12195175 | VERIFIED |
| Speech production unaffected until delay exceeds | 30 | ms | [21] | Abstract | same | VERIFIED |
| Permissible limit stated | 20 to 30 | ms | [21] | Abstract, CONCLUSIONS | same | VERIFIED |
| Across-frequency delay significantly more disturbing than 0 ms / mean "disturbing" | 9 / about 20 | ms | Stone & Moore 2003, Ear Hear 24(2):175-183 [22] | Abstract | PMID 12677113 | VERIFIED |
| VCV identification drops significantly once across-frequency delay reaches | 15 | ms | [22] | Abstract | same | VERIFIED |
| Hearing-impaired talkers: rating 3 ("probably just acceptable") for delays of | 14 to 30 | ms | Stone & Moore 2005, Ear Hear 26(2):225-235 [23] | Abstract, CONCLUSIONS | PMID 15809547 | VERIFIED |
| Delays tested in 2005 | 13 / 21 / 30 / 40 | ms | [23] | Abstract, DESIGN | same | VERIFIED |
| One-way delay upper bound for general network planning | 400 | ms | ITU-T G.114 (05/2003) [24] | Sec. 4 | https://www.itu.int/rec/T-REC-G.114 | VERIFIED |
| Mouth-to-ear delay below which most applications get "essentially transparent interactivity" | 150 | ms | [24] | Sec. 4 | same | VERIFIED |
| Highly interactive tasks may be affected below | 100 | ms | [24] | Sec. 2 / Summary | same | VERIFIED |
| Sidetone must be provided via earphones, in phase, not filtered or modified | requirement | - | MIL-STD-1472G CHG-1 [5] | 5.3.1.6 j, p. 126 | everyspec | VERIFIED |
| Binaural asynchronous delay not allowed above (critical voice comms) | 1 | ms | [5] | 5.3.1.6 i | same | VERIFIED |
| Headset output peak voice level at the ear, maximum | 115 | dB peak | [5] | 5.3.1.8.2 a(3), p. 130 | same | VERIFIED |
| Comms output at least this much above the anticipated ambient | 15 | dB | [5] | 5.3.1.8.2 a(2) | same | VERIFIED |
| Receiver/headset frequency response | +/-3 dB, 250 to 6000 Hz | - | [5] | 5.3.1.8.2 b | same | VERIFIED |
| AGC allowed if the speech-to-noise differential is not greater than | 20 | dB | [5] | 5.3.1.6 h | same | VERIFIED |
| ITU-T P.310 (03/2003): STMR range changed from 10-15 dB to | 10 to 20 | dB | ITU-T P.310 summary [25] | Summary page | https://www.itu.int/rec/T-REC-P.310 | UNVERIFIED (search snippet) |
| 3GPP TS 26.131 STMR: >= 15 dB (preferably <= 23 dB) at nominal volume; >= 10 dB at other settings; sidetone delay clause 7.5.2 | dB | - | 3GPP TS 26.131 cl. 7.5 [26] | 7.5.1 / 7.5.2 | https://www.3gpp.org/DynaReport/26131.htm | UNVERIFIED (clause body not read) |
| ITU-T G.131 scope | talker echo control only; no sidetone limits found | - | ITU-T G.131 summary | - | https://www.itu.int/rec/T-REC-G.131 | VERIFIED (scope from ITU summary) |

---

## 8. MIL-STD-1275E (22 Mar 2013): 28 V DC input power [27]

Note: 1275E has a single envelope. It does not split limits into "normal / generator-only / battery-only" modes the way 1275D (2006) did. 1275D mode values were not read: MISSING.

| quantity | value | unit | source | where in source | URL | status |
|---|---|---|---|---|---|---|
| Steady-state operating range, including ripple | 20 to 33 | V DC | [27] | 5.1.1.1, p. 7 | https://everyspec.com/MIL-STD/MIL-STD-1100-1299/MIL-STD-1275E_45886/ | VERIFIED |
| Ripple limit | per MIL-STD-461 CS101, Fig. CS101-1, with the 150 kHz value extended to 250 kHz | - | [27] | 5.1.1.2 | same | VERIFIED |
| Ripple test nominal voltages | 23 and 30 | V DC | [27] | 5.3.1.2 | same | VERIFIED |
| Initial engagement surge (IES): minimum voltage / maximum duration / minimum spacing | 12 V / 1 s / 1 s | - | [27] | 5.1.2.1; Fig. 6 p. 8 | same | VERIFIED |
| Cranking: minimum voltage / maximum duration | 16 V / 30 s | - | [27] | 5.1.2.2; Fig. 6 | same | VERIFIED |
| Injected spike envelope | +/-250 V for 70 us, decaying to 100 V (upper) and 18 V (lower) at 1 ms | - | [27] | Fig. 7, p. 9 (labels "250V, 70us", "-250V, 70us", "100V, 1 ms", "18V, 1 ms") | same | VERIFIED |
| Injected spike: maximum rise time / maximum energy | 50 ns / 2 J | - | [27] | 5.1.3.1.1 | same | VERIFIED |
| Spike test: count / interval / ringing frequency | >= 50 per polarity / 1 s / 100 to 500 kHz | - | [27] | 5.3.3.1.1 | same | VERIFIED |
| Emitted spike energy, maximum | 125 | mJ | [27] | 5.1.3.1.2; 5.3.3.1.2 | same | VERIFIED |
| Surge envelope (upper) | 100 V for 50 ms, then 33 V at 500 ms | - | [27] | Fig. 8, p. 10 | same | VERIFIED |
| Surge envelope (lower) | 18 V for 500 ms, then 20 V at 600 ms | - | [27] | Fig. 8 | same | VERIFIED |
| Injected surge: maximum energy / source impedance | 60 J / 500 mOhm nominal | - | [27] | 5.1.3.2.1; Fig. 8 note | same | VERIFIED |
| Surge test parameters: Vnom / Vpeak / rise / duration / Rs / pulses / spacing | 30 V (-0/+1) / 100 V (-0/+10) / 1 < t < 10 ms / 50 ms (-0/+5) / 500 mOhm (-25/+0) / 5 / 5 s | - | [27] | Table I, p. 14 | same | VERIFIED |
| Reverse polarity: equipment must protect itself; reverse current <= normal operating current | requirement | - | [27] | 4.1, p. 6-7 | same | VERIFIED |
| Reverse polarity test voltage and duration | not specified in 1275E text read | - | [27] | - | - | VERIFIED (absence in 4.1) |
| Test ambient | +23 +/- 5 C, 0-90 % RH, 80-102 kPa | - | [27] | 5.2.1 | same | VERIFIED |
| Protection-circuit output for 28 V designs during spike/surge (design target) | <= 34 V | V | TI TIDUDG1 (2022) reference design [28] | - | https://www.ti.com/lit/pdf/tidudg1 | UNVERIFIED (snippet) |

---

## 9. MIL-STD-461G (11 Dec 2015) [29]

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Table V, "Ground, Army" applicability: CE101 -, CE102 A, CE106 L, CS101 A, CS103 S, CS104 S, CS105 S, CS109 -, CS114 A, CS115 A, CS116 A, CS117 S, CS118 A, RE101 -, RE102 A, RE103 L, RS101 L, RS103 A, RS105 - | A = applicable, L = limited, S = procuring activity specifies, - = blank | - | [29] | Table V, p. 26 (column mapping checked with word coordinates) | https://everyspec.com/MIL-STD/MIL-STD-0300-0499/MIL-STD-461G_53571/ | VERIFIED |
| CE102 range | 10 kHz to 10 MHz | - | [29] | 5.5.1 | same | VERIFIED |
| CE102 basic curve (28 V) starts at | 94 | dBuV | [29] | Fig. CE102-1, p. 37 (labels "94", "28V BASIC CURVE"; relaxations 115 V 6 dB, 220 V 9 dB, 270 V 10 dB, 440 V 12 dB) | same | VERIFIED |
| CE102 basic curve: falls to 60 dBuV at 500 kHz, flat to 10 MHz | 60 | dBuV | [29] | Fig. CE102-1 (break point not legible in the text layer) | same | UNVERIFIED (memory) |
| CS101 range (DC equipment) | 30 Hz to 150 kHz | - | [29] | 5.7.1 | same | VERIFIED |
| CS101 voltage limit, curve #2 (EUT <= 28 V) | 126 dBuV (about 2 V rms) at low frequency, falling to 96.5 dBuV at 150 kHz | dBuV | [29] | Fig. CS101-1, p. 50 (labels 136/126 and 106.5/96.5) | same | VERIFIED (figure labels) |
| CS101 power limit | 80 W at low frequency, falling to 0.09 W at 150 kHz | W | [29] | Fig. CS101-2, p. 51 | same | VERIFIED (figure labels) |
| CS101 low-frequency break point | 5 kHz | Hz | - | Fig. CS101-1 | - | UNVERIFIED (memory) |
| CS114 range | 10 kHz to 200 MHz | - | [29] | 5.12.1 | same | VERIFIED |
| CS114 curve, Army ground: 10 kHz-2 MHz / 2-30 MHz / 30-200 MHz | Curve 3 / Curve 4 / Curve 4 | - | [29] | Table VI, p. 66 | same | VERIFIED |
| CS114 flat-region current: Curve 3 / Curve 4 | 95 / 103 | dBuA | [29] | 5.12.2 | same | VERIFIED |
| CS115 pulse: amplitude / rise and fall / width / rate / duration | 5 A / 2 ns / 30 ns min / 30 Hz / 1 min | - | [29] | 5.13.2; Fig. CS115-1, p. 74 | same | VERIFIED |
| CS115 applicability | all aircraft, space and ground system interconnecting cables, including power | - | [29] | 5.13.1 | same | VERIFIED |
| CS116 range / test frequencies | 10 kHz-100 MHz / 0.01, 0.1, 1, 10, 30, 100 MHz | - | [29] | 5.14.1-5.14.2 | same | VERIFIED |
| CS116 maximum current | 10 | A peak | [29] | App. A (attenuator note, PDF p. 254: "the 10 A current limit for CS116") | same | VERIFIED |
| CS116 waveform damping factor Q / repetition / duration | 15 +/- 5 / 0.5 to 1 pulse/s / 5 min | - | [29] | Fig. CS116-1; 5.14.2 | same | VERIFIED |
| RE102 ground range | 2 MHz to 18 GHz | - | [29] | 5.18.1 a | same | VERIFIED |
| RE102 ground limit, "Navy Mobile & Army" curve | 44 dBuV/m at the low end, rising to 89 dBuV/m at 18 GHz | dBuV/m | [29] | Fig. RE102-4, p. 117 (labels 44/89 vs 24/69 for Navy Fixed & Air Force) | same | VERIFIED (figure labels); curve shape between labels UNVERIFIED |
| RS103 Army ground field strength: 2-30 MHz / 30 MHz-1 GHz / 1-18 GHz / 18-40 GHz | 50 / 50 / 50 / 50 | V/m | [29] | Table XI, p. 145 | same | VERIFIED |
| RS103 ranges: 2-30 MHz (Army, Navy), 30 MHz-18 GHz (all), 18-40 GHz optional | - | - | [29] | 5.21.1 | same | VERIFIED |

---

## 10. MIL-STD-810H (31 Jan 2019) methods for a vehicle-mounted crew box

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| Method list: 500.6 Low Pressure (Altitude); 501.7 High Temperature; 502.7 Low Temperature; 503.7 Temperature Shock; 504.3 Contamination by Fluids; 505.7 Solar Radiation; 506.6 Rain; 507.6 Humidity; 508.8 Fungus; 509.7 Salt Fog; 510.7 Sand and Dust; 511.7 Explosive Atmosphere; 512.6 Immersion; 513.8 Acceleration; 514.8 Vibration; 515.8 Acoustic Noise; 516.8 Shock; 517.3 Pyroshock; 518.2 Acidic Atmosphere; 519.8 Gunfire Shock; 520.5 Combined Environments; 521.4 Icing/Freezing Rain; 522.4 Ballistic Shock; 523.4 Vibro-Acoustic/Temperature; 524.1 Freeze/Thaw; 525.2 Time Waveform Replication; 526.2 Rail Impact; 527.2 Multi-Exciter; 528.1 Mechanical Vibrations of Shipboard Equipment | list | - | DSIAC course page [30] | method list | https://dsiac.dtic.mil/events/military-standard-810h-mil-std-810h-testing-nts-fullerton-ca/ | VERIFIED (secondary list, not the standard). Note: one search source gave 522.2 for Ballistic Shock |
| Basic Hot (A2): ambient / induced (storage, transit) | 30 to 43 / 30 to 63 | C | MIL-STD-810H Method 501.7 [31] | Table 501.7-I, p. 501.7-4 | (vendor-hosted copy of the DLA ASSIST file) mil-std-810h_high_temp.pdf (copy of the DLA ASSIST file) | VERIFIED |
| Hot Dry (A1): ambient / induced | 32 to 49 / 33 to 71 | C | [31] | Table 501.7-I | same | VERIFIED |
| Basic Cold (C1): ambient / induced | -21 to -32 / -25 to -33 | C | MIL-STD-810H Method 502.7 [32] | Table 502.7-I | (vendor-hosted copy of the DLA ASSIST file) mil-std-810h-low-temp.pdf | VERIFIED |
| Cold (C2): ambient / induced | -37 to -46 / -37 to -46 | C | [32] | Table 502.7-I | same | VERIFIED |
| Severe Cold (C3): ambient / induced | -51 / -51 | C | [32] | Table 502.7-I | same | VERIFIED |
| 514.8 vibration: ground-vehicle category used by an industry vehicle-intercom datasheet: "wheeled, tracked, aggravated and IFV profiles per MIL-STD-810G Method 514.6, Proc I, category 20" | example | - | Vehicle intercom vendor datasheet (name withheld) [33] | Environmental table | (URL withheld: vendor) | VERIFIED (vendor practice, 810G) |
| Same datasheet: operating / storage temperature | -46 to +71 / -51 to +71 | C | [33] | Environmental table | same | VERIFIED (vendor) |
| Same datasheet: EMI set | MIL-STD-461F CE102, CS101, CS114, RE102, RS103, CS115; HEMP RS105, CS116 | - | [33] | Environmental table | same | VERIFIED (vendor) |
| Same datasheet: salt fog 48 h (509.5); dust 20 +/- 3 mph for 12 h (510.5 Proc I) | - | - | [33] | Environmental table | same | VERIFIED (vendor, 810G) |
| 514.8 Category 4 composite wheeled vehicle: test time per axis | 32 or 40 | min/axis | vendor test reports (conflicting) | - | - | UNVERIFIED (conflicting secondary) |
| 516.8 functional shock (ground equipment) 40 g / 11 ms terminal-peak sawtooth; crash hazard 75 g | - | - | - | - | - | UNVERIFIED (memory) |
| 509.7 salt fog 5 % NaCl, 35 C, 24 h wet / 24 h dry x 2; 512.6 immersion 1 m for 30 min; 510.7 blowing dust 10.6 g/m3, blowing sand 2.2 g/m3 | - | - | - | - | - | UNVERIFIED (memory; not found in open text) |

---

## 11. MIL-HDBK-217F Notice 2 (28 Feb 1995) [34]

All generic failure rates lambda_g are in failures per 10^6 h, from Appendix A "Parts Count" (Notice 2 pages), read from rendered page images. Default temperatures: microcircuits GB 50 C / GF 60 C / GM 65 C junction; passives GB 30 C / GF 40 C / GM 45 C ambient. Quality factor pi_Q must be applied separately.

| quantity | value (GB / GF / GM) | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| pi_E resistors (Sec. 9.1) | 1.0 / 4.0 / 16 | - | [34] | p. 9-3 "Environment Factor" | https://www.dinfo.unifi.it/upload/sub/laboratori/labmaq/labme_1/mil_hdbk_217f_2.pdf | VERIFIED |
| pi_E capacitors (Sec. 10.1) | 1.0 / 10 / 20 | - | [34] | p. 10-5 | same | VERIFIED |
| pi_E microcircuits | 0.5 / 2.0 / 4.0 (NS 4.0, NU 6.0, AIC 4.0, AIF 5.0, AUC 5.0, AUF 8.0, ARW 8.0, SF 0.5, MF 5.0, ML 12, CL 220) | - | sqconline "MIL-HDBK-217 Microcircuit Tables" [35] | pi_E table | https://www.sqconline.com/node/82 | VERIFIED (secondary; 217F base Sec. 5.10 not in the N2 file) |
| MOS microprocessor, up to 8 / 16 / 32 bits | 0.048/0.089/0.13 ; 0.093/0.17/0.24 ; 0.19/0.34/0.49 | per 10^6 h | [34] | App. A p. A-2 | same | VERIFIED |
| MOS gate/logic array, digital, 10,001-30,000 gates (180-pin PGA) (FPGA/ASIC proxy) | 0.084 / 0.22 / 0.39 | per 10^6 h | [34] | p. A-2 | same | VERIFIED |
| MOS linear, 101-300 transistors (op-amp proxy) | 0.017 / 0.041 / 0.065 | per 10^6 h | [34] | p. A-2 | same | VERIFIED |
| MOS linear, 301-1000 transistors (audio codec analogue part, proxy) | 0.033 / 0.074 / 0.11 | per 10^6 h | [34] | p. A-2 | same | VERIFIED |
| MOS linear, 1001-10,000 transistors | 0.05 / 0.12 / 0.18 | per 10^6 h | [34] | p. A-2 | same | VERIFIED |
| Resistor, film chip RM (MIL-R-55342) | 0.0037 / 0.016 / 0.07 | per 10^6 h | [34] | p. A-7 | same | VERIFIED |
| Resistor quality pi_Q: S / R / P / M / MIL-SPEC / lower | 0.030 / 0.10 / 0.30 / 1.0 / 3.0 / 10 | - | [34] | p. A-7; p. 9-3 | same | VERIFIED |
| Capacitor, ceramic chip CDR (MIL-C-55681) | 0.0035 / 0.053 / 0.13 | per 10^6 h | [34] | p. A-8 | same | VERIFIED |
| Capacitor, ceramic general purpose CK/CKR | 0.0017 / 0.026 / 0.064 | per 10^6 h | [34] | p. A-8 | same | VERIFIED |
| Capacitor, tantalum chip CWR (MIL-C-55365) | 0.00014 / 0.0016 / 0.0036 | per 10^6 h | [34] | p. A-8 | same | VERIFIED |
| Capacitor, tantalum solid CSR | 0.0014 / 0.017 / 0.037 | per 10^6 h | [34] | p. A-8 | same | VERIFIED |
| Capacitor, aluminium oxide (electrolytic) CU/CUR | 0.0013 / 0.019 / 0.047 | per 10^6 h | [34] | p. A-8 | same | VERIFIED |
| Capacitor quality pi_Q: D / C / S,B / R / P / M / L / MIL-SPEC / lower | 0.001 / 0.01 / 0.03 / 0.1 / 0.3 / 1.0 / 1.5 / 3.0 / 10 | - | [34] | p. 10-5 | same | VERIFIED |
| Transformer, switching (MIL-T-21038) | 0.00061 / 0.0042 / 0.0090 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Transformer, audio (MIL-T-27) | 0.015 / 0.10 / 0.22 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Coil, fixed inductor or choke | 0.000032 / 0.00022 / 0.00047 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Relay, general purpose (balanced armature) | 0.049 / 0.12 / 1.0 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Relay, solid state | 0.029 / 0.087 / 0.35 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Connector, circular (per connector, not per pin) | 0.0011 / 0.0013 / 0.011 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Connector, rectangular | 0.050 / 0.060 / 0.52 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| Switch, pushbutton / rotary | 0.10/0.30/1.8 ; 0.11/0.33/2.0 | per 10^6 h | [34] | p. A-9 | same | VERIFIED |
| DC-DC converter (hybrid/module) | no Appendix A row; needs a hybrid model (Sec. 5.x) or vendor MTBF | - | - | - | - | MISSING |
| Telcordia SR-332 / FIDES alternative values | not collected | - | - | - | - | MISSING |

---

## 12. Bus and intercom technology

| quantity | value | unit | source | where | URL | status |
|---|---|---|---|---|---|---|
| AVB SR Class A latency bound | 2 ms over max 7 hops | ms | Wikipedia "Audio Video Bridging" [36] (claim uncited in the article); primary IEEE 802.1BA-2011 (superseded by 802.1BA-2021) | Summary | https://en.wikipedia.org/wiki/Audio_Video_Bridging | VERIFIED (secondary, uncited) / primary UNVERIFIED |
| AVB SR Class B latency bound | 50 ms over 7 hops | ms | [36] | Summary | same | VERIFIED (secondary) |
| Class A / Class B observation interval | 125 / 250 | us | [36] | Summary | same | VERIFIED (secondary) |
| Maximum reservable bandwidth for SR classes (802.1Qav default) | 75 % | - | - | - | - | UNVERIFIED (memory) |
| Example of a fielded digital vehicle intercom: Ethernet/VoIP based, power and Ethernet in one cable, up to 16 crew members, ANR headset support, J1939/CAN interfaces, codecs for low-bandwidth slip rings | example | - | [33] | Key features | vendor datasheet | VERIFIED (vendor) |
| 1970s military crew intercom study: TDM signalling between crew stations with CVSD voice at 40 kb/s | 40 | kb/s | DTIC ADA087830 | - | https://apps.dtic.mil/sti/tr/pdf/ADA087830.pdf | UNVERIFIED (search snippet; DTIC offline) |
| TDM digital matrix intercom architecture (patent) | architecture reference | - | US 6,069,878 | - | https://patents.justia.com/patent/6069878 | UNVERIFIED (not read) |
| RS-485 TDM intercom physical layer example | - | - | - | - | - | MISSING |
| Cable mass: analogue multi-pair audio harness vs Ethernet | - | - | - | - | - | MISSING |

---

## Gaps still open

1. Octave-band dB(C) values for vehicles. Can be DERIVED from 1c, but no published value was found.
2. IEC 60268-16 Annex F qualification bands (A+..U): the values are from memory only.
3. MIL-STD-1472H table number and wording (1472G CHG-1 used instead).
4. ISO 9921:2003 Annex F text.
5. MIL-STD-810H 514.8 ground-vehicle schedules, 516.8 shock, 509/510/512 parameters (open text not read).
6. MIL-STD-1275D mode values.
7. DTIC/ARL/HEL Garinther-era vehicle noise surveys (DTIC was offline).
8. ANSI S12.6 REAT data for a generic circumaural headset.
9. Telcordia/FIDES rates; DC-DC converter failure rate.
10. RS-485 TDM example; cable mass.
11. CE102 and CS101 break points; STMR values from the primary ITU/3GPP text.

---

## References (IEEE style)

[1] D. G. Zimcik, R. M. Provencher, B. Lecheminant, and D. McNamara, "Measurement of noise in armoured personnel carriers," in *Habitability of Combat and Transport Vehicles: Noise, Vibration and Motion*, NATO RTO-MP-AVT-110, paper 9, Prague, Czech Republic, Oct. 2004. [Online]. Available: https://publications.sto.nato.int/publications/STO%20Meeting%20Proceedings/RTO-MP-AVT-110/MP-AVT-110-09.pdf (also DTIC ADA446765)

[2] K. Buck, P. Hamery, and V. Zimpfer, "The European regulation 2003/10/EC and the impact of its application to the military noise exposure," in *Proc. 20th Int. Congr. Acoustics (ICA 2010)*, Sydney, Australia, Aug. 2010, paper p495. [Online]. Available: https://www.acoustics.asn.au/conference_proceedings/ICA2010/cdrom-ICA2010/papers/p495.pdf

[3] S. J. van Wijngaarden and S. James, "Protecting crew members against military vehicle noise," in NATO RTO-MP-AVT-110, keynote KN1, Prague, Oct. 2004. [Online]. Available: https://publications.sto.nato.int/publications/STO%20Meeting%20Proceedings/RTO-MP-AVT-110/MP-AVT-110-$KN1.pdf (also DTIC ADA446747)

[4] *Department of Defense Design Criteria Standard: Noise Limits*, MIL-STD-1474E, U.S. DoD, 15 Apr. 2015. [Online]. Available: https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1474E_52224/

[5] *Department of Defense Design Criteria Standard: Human Engineering*, MIL-STD-1472G w/Change 1, U.S. DoD, 17 Jan. 2019. [Online]. Available: https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1472G_CHG-1_56051/

[6] *Department of Defense Design Criteria Standard: Human Engineering*, MIL-STD-1472H, U.S. DoD, 15 Sep. 2020. [Online]. Available: https://everyspec.com/MIL-STD/MIL-STD-1400-1499/MIL-STD-1472H_57041/ (not read)

[7] NTi Audio, "How do we measure speech intelligibility (STI)?," know-how page. [Online]. Available: https://www.nti-audio.com/en/support/know-how/how-do-we-measure-speech-intelligibility-sti (accessed 9 Nov. 2026)

[8] Wikipedia, "Speech transmission index." [Online]. Available: https://en.wikipedia.org/wiki/Speech_transmission_index (accessed 9 Nov. 2026)

[9] *Ergonomics — Assessment of Speech Communication*, ISO 9921:2003, International Organization for Standardization, 2003.

[10] *Sound System Equipment — Part 16: Objective Rating of Speech Intelligibility by Speech Transmission Index*, IEC 60268-16:2020, ed. 5.0, IEC, 2020 (preview pages via iTeh Standards). [Online]. Available: https://cdn.standards.iteh.ai/samples/21925/7a5eb0d5e54e49e685b89ede43782760/IEC-60268-16-2020.pdf

[11] Architectural Institute of Japan (AIJ), "Benchmark problems for acoustical parameters: Basic B-32 and A-21 (STI, complying with IEC 60268-16:2020 Edition 5)." [Online]. Available: https://news-sv.aij.or.jp/kankyo/s24/benchmark/b32/b32.html and https://news-sv.aij.or.jp/kankyo/s24/benchmark/a21/a21.html

[12] The MathWorks, "Speech transmission index IEC 60268-16 edition 5 compliance," Audio Toolbox documentation. [Online]. Available: https://au.mathworks.com/help/audio/ug/speech-transmission-index-iec60268-16-edition-5-compliance.html

[13] P. Rajmic, J. Schimmel, and Š. Cieslar, "A MATLAB toolbox for computation of speech transmission index (STI)," arXiv:2510.03825 [eess.AS], Oct. 2025.

[14] "SII: Calculate ANSI S3.5-1997 speech intelligibility index," R package documentation (data objects `octave`, `overall.spl`). [Online]. Available: https://rdrr.io/rforge/SII/man/critical.html

[15] P. Bottalico, I. I. Passione, S. Graetzer, and E. J. Hunter, "Evaluation of the starting point of the Lombard effect," *Acta Acust. united Acust.*, vol. 103, no. 1, pp. 169–172, 2017. [Online]. Available: https://pmc.ncbi.nlm.nih.gov/articles/PMC5612409

[16] K. S. Pearsons, R. L. Bennett, and S. Fidell, "Speech levels in various noise environments," U.S. EPA, Rep. EPA-600/1-77-025, May 1977 (not read).

[17] *Helmet, Combat Vehicle Crewman, Ballistic Shell*, MIL-DTL-44117B, U.S. DoD (eMilSpec reproduction, page 13). [Online]. Available: https://armorpersonal.emilspec.com/MIL-DTL-44117/page13.html

[18] Vendor product data, pressure-gradient dynamic boom microphone, 150 ohm (name and URL withheld under the repository naming rule).

[19] "Noise control device for a boom mounted noise-canceling microphone," U.S. Patent 6 009 184 (not opened).

[20] M. A. Stone and B. C. J. Moore, "Tolerable hearing aid delays. I. Estimation of limits imposed by the auditory path alone using simulated hearing losses," *Ear Hear.*, vol. 20, no. 3, pp. 182–192, Jun. 1999, doi: 10.1097/00003446-199906000-00002.

[21] M. A. Stone and B. C. J. Moore, "Tolerable hearing aid delays. II. Estimation of limits imposed during speech production," *Ear Hear.*, vol. 23, no. 4, pp. 325–338, Aug. 2002, doi: 10.1097/00003446-200208000-00008.

[22] M. A. Stone and B. C. J. Moore, "Tolerable hearing aid delays. III. Effects on speech production and perception of across-frequency variation in delay," *Ear Hear.*, vol. 24, no. 2, pp. 175–183, Apr. 2003, doi: 10.1097/01.AUD.0000058106.68049.9C.

[23] M. A. Stone and B. C. J. Moore, "Tolerable hearing-aid delays: IV. Effects on subjective disturbance during speech production by hearing-impaired subjects," *Ear Hear.*, vol. 26, no. 2, pp. 225–235, Apr. 2005, doi: 10.1097/00003446-200504000-00009.

[24] *One-Way Transmission Time*, ITU-T Rec. G.114, May 2003. [Online]. Available: https://www.itu.int/rec/T-REC-G.114

[25] *Transmission Characteristics for Telephone Band (300-3400 Hz) Digital Telephones*, ITU-T Rec. P.310, Mar. 2003 (summary only).

[26] *Terminal Acoustic Characteristics for Telephony; Requirements*, 3GPP TS 26.131, Rel-19 (clause 7.5 not read).

[27] *Department of Defense Interface Standard: Characteristics of 28 Volt DC Input Power to Utilization Equipment in Military Vehicles*, MIL-STD-1275E, U.S. DoD, 22 Mar. 2013. [Online]. Available: https://everyspec.com/MIL-STD/MIL-STD-1100-1299/MIL-STD-1275E_45886/

[28] Texas Instruments, "TIDA-010242 reference design," TIDUDG1, Jul. 2022 (snippet only).

[29] *Department of Defense Interface Standard: Requirements for the Control of Electromagnetic Interference Characteristics of Subsystems and Equipment*, MIL-STD-461G, U.S. DoD, 11 Dec. 2015. [Online]. Available: https://everyspec.com/MIL-STD/MIL-STD-0300-0499/MIL-STD-461G_53571/

[30] DSIAC, "Military Standard 810H (MIL-STD-810H) testing, NTS Fullerton, CA," course listing. [Online]. Available: https://dsiac.dtic.mil/events/military-standard-810h-mil-std-810h-testing-nts-fullerton-ca/

[31] *Environmental Engineering Considerations and Laboratory Tests*, MIL-STD-810H, Method 501.7 High Temperature, U.S. DoD, 31 Jan. 2019 (ASSIST copy, downloaded 2020-05-04, vendor-hosted copy). [Online]. Available: (vendor-hosted copy of the DLA ASSIST file) mil-std-810h_high_temp.pdf

[32] MIL-STD-810H, Method 502.7 Low Temperature, U.S. DoD, 31 Jan. 2019 (ASSIST copy, vendor-hosted). [Online]. Available: (vendor-hosted copy of the DLA ASSIST file) mil-std-810h-low-temp.pdf

[33] Vendor datasheet of a fielded digital vehicle intercom, 2019 (name withheld). [Online]. Available: (URL withheld: vendor)

[34] *Reliability Prediction of Electronic Equipment*, MIL-HDBK-217F Notice 2, U.S. DoD, 28 Feb. 1995. [Online]. Available: https://www.dinfo.unifi.it/upload/sub/laboratori/labmaq/labme_1/mil_hdbk_217f_2.pdf

[35] SQC Online, "MIL-HDBK-217 microcircuit tables." [Online]. Available: https://www.sqconline.com/node/82

[36] Wikipedia, "Audio Video Bridging." [Online]. Available: https://en.wikipedia.org/wiki/Audio_Video_Bridging (accessed 9 Nov. 2026)
