# Open items: what only hardware, a laboratory or a user can close

Everything that software or simulation could close was closed in Rev B (see CHANGELOG.md). The items below
need hardware, a laboratory, real crews or measured data. Each names the requirement it blocks and what
would close it.

| # | Item | Blocks | What closes it |
|---|---|---|---|
| H1 | Real codec / SAI driver and timing on the RT1064 board (Renode has no SAI model) | CB-05 WCET, SYS-3.2.4 measured | Bring-up on a MIMXRT1064-EVK with its codec; measure the frame time with DWT and the loop-back latency |
| H2 | Analog sidetone through the codec's microphone-to-headphone bypass (decision D4) | SYS-3.3.2 | Codec register setup and a measured sidetone delay below 1 ms on the board |
| H3 | Boom microphone and noise shield with ≥ 10 dB rejection in 1–4 kHz | SYS-3.2.1 objective, SYS-3.3.3 | Measure candidate headsets in a diffuse-field noise room (the transmit SNR is what limits STI, DOSSIER §9.2) |
| H4 | Line width of real track tones, and a sprocket-phase signal from the vehicle | tonal ANR performance (DOSSIER §15 Q1) | One recording at the error microphone, made together with the sprocket pick-up on a moving vehicle |
| H5 | Hearing protection above 108 dB(A) for 8 h (decision D3: earplug) | SYS-3.2.2 | User decision on communication earplugs or exposure time; REAT data for the chosen earplug under the helmet |
| H6 | Relay operate and release times, and the charge-pump hold time | SYS-3.2.7 (the 50 ms budget includes the relay) | Bench measurement on the chosen relay and drive circuit |
| H7 | Emergency-line loopback wiring and the BIT coverage of the crew box | SYS-3.2.8 (coverage decides the number, DOSSIER §10) | Schematic review and a fault-injection campaign on hardware |
| H8 | 28 V input stage | SYS-3.4.1 | MIL-STD-1275E surge, spike, IES and cranking tests on a built input stage |
| H9 | EMC and environment qualification | SYS-3.5.1, SYS-3.5.2 | Accredited laboratory: MIL-STD-461G and MIL-STD-810H |
| H10 | STI with listeners | validation (DOSSIER §13.4) | MRT (ANSI S3.2) with crews in a noise chamber at the recorded vehicle spectrum |
| H11 | CCU hardware: radio interfaces, 28 V, enclosure (its core logic is built and tested) | SYS-3.3.x on hardware | CCU board design and integration with real radios (integration points IP4/IP5) |
