| # | Item | Failure mode | Local effect | System effect | Detection | Compensating provision | Sev | Prob | Risk |
|---|---|---|---|---|---|---|---|---|---|
| 1 | CCU | dead (power, processor) | no downlink frames | all boxes lose intercom/radio | link supervision: no frame for 20 ms | all boxes -> BYPASS, analog party line | 2 | D | Medium |
| 2 | bus link | open cable / connector | one box loses frames | that crew member isolated digitally | link supervision | box -> BYPASS; CCU sees missing uplink, commands others to keep digital | 3 | D | Medium |
| 3 | bus link | noise bursts (EMI) | corrupted frames | clicks, dropouts | CRC-16, lost-frame counter | concealment <= 3 frames, then mute; BYPASS if > 20 ms | 4 | C | Low |
| 4 | crew box | processor hang | relay drive stops toggling | box silent | dynamic relay drive (charge pump) | relay drops -> BYPASS within relay release time | 3 | D | Medium |
| 5 | crew box | codec failure | no audio to/from headset | crew member loses digital path | BIT: codec loop-back | latched BYPASS | 3 | D | Medium |
| 6 | crew box | local supply out of range | erratic audio | as above | BIT: supply monitor | latched BYPASS | 3 | D | Medium |
| 7 | crew box | ANR divergence (wrong S_hat) | loud low-frequency tone | hearing hazard, masking | error-power guard (+6 dB for 0.2 s), weight guard | ANR off -> DEGRADED, output clamped | 3 | D | Medium |
| 8 | crew box | bypass relay stuck energised | no emergency path | latent until needed | maintenance test of bypass (MAINT) | none in operation: latent failure | 2 | E | Medium |
| 9 | crew box | emergency amplifier dead | no audio in BYPASS | latent until needed | maintenance test of bypass | none in operation: latent failure | 3 | D | Medium |
| 10 | headset | mic open / short | no transmit | crew member cannot talk | BIT: mic bias current | report to crew/CCU; spare headset | 3 | C | Medium |
| 11 | headset | earphone open | no receive | crew member cannot hear | BIT: earphone impedance | spare headset | 3 | C | Medium |
| 12 | power 28 V | 1275E surge 100 V / 50 ms | input stage stress | box reset if unprotected | design: surge stopper, see power study | rides through | 3 | C | Medium |
| 13 | power 28 V | cranking dip to 16 V / 30 s; IES 12 V / 1 s | supply droop | reset during engine start | design: wide-input converter + hold-up | rides through | 3 | B | Serious |
