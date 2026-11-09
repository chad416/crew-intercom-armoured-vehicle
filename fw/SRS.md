# Crew-box software requirements (SRS)

Rev A, 9 Nov 2026. Derived from the system requirements in [DOSSIER.md §5](../DOSSIER.md) (SYS-x.y.z).
Statement form after Wasson 2016 ch. 22: subject, shall, outcome, condition. `fw/tools/trace.py` checks
that every requirement below appears in [docs/traceability.md](docs/traceability.md) with at least one
existing test.

## 1. Audio frame

- **CB-01 (SYS-3.2.1):** The crew box shall process audio at 16 kHz (wideband).
  - Rationale: S1 shows that 32 kHz adds 0.006 STI at 111 dB(A) (DOSSIER §9.2).
  - Narrowband (300–3400 Hz) costs 0.02–0.05 STI.
- **CB-02 (SYS-3.2.4):** The crew box shall process audio in frames of 1 ms (16 samples).
  - Rationale: the S4 frame-length sensitivity. 2 ms frames leave 1.6 ms of sidetone margin; 4 ms
    frames break the 10 ms limit.
- **CB-03 (SYS-3.2.4):** The crew box shall generate sidetone locally from its own microphone, never via
  the bus.
- **CB-04 (SYS-3.2.7):** In BYPASS the crew box shall keep sending telemetry-only uplink frames, so that
  the CCU still sees its state.
- **CB-05 (SYS-3.2.4):** The worst-case processing time of one frame shall be below 0.5 ms (50 % of the
  frame).

## 2. Bus frames

- **CB-10 (SYS-3.2.7):** The crew box shall frame bus messages as `A5 5A len type payload CRC16`, with
  CRC-16/CCITT-FALSE over len, type and payload.
- **CB-11 (SYS-3.2.7):** The crew box shall decode downlink frames carrying 0 to 4 audio channels of 16
  samples, and uplink frames carrying 0 or 1 channel.
- **CB-12 (SYS-3.5.1):** The crew box shall reject and count any frame with a CRC error, an unknown type
  or an inconsistent length. It shall resynchronise on the next sync pair.
- **CB-13 (SYS-3.2.7):** The crew box shall ignore uplink frames from other boxes seen on its port.

## 3. Link supervision

- **CB-14 (SYS-3.2.7):** The crew box shall count consecutive ticks with and without a valid downlink
  frame.
  - It shall count lost frames from the sequence-number gaps, once per frame (IT04, Rev A fix).
- **CB-15 (SYS-3.5.1):** For a missing frame the crew box shall repeat the last frame at −6 dB per
  repetition for up to 3 frames, then mute.
- **CB-16 (SYS-3.2.7):** A sequence-number wrap (255 → 0) shall not count as a loss.

## 4. Operating states

- **CB-20 (SYS-3.2.7):** From INIT, the crew box shall enter NORMAL on the first valid frame when no
  critical BIT fault is present. It shall enter DEGRADED instead if the ANR is faulty.
- **CB-21 (SYS-3.2.7):** If no valid frame arrives within 2 s in INIT, the crew box shall enter BYPASS
  (not latched).
- **CB-22 (SYS-3.2.7):** From NORMAL, DEGRADED or MAINT, the crew box shall enter BYPASS after 20
  consecutive ticks without a valid frame.
- **CB-23 (SYS-3.2.7):** The crew box shall leave a non-latched BYPASS only after 2000 consecutive ticks
  with a valid frame.
- **CB-24 (SYS-3.2.7):** A critical BIT fault (codec, supply) shall put the crew box in BYPASS from any
  state. That BYPASS shall be latched until power-off.
- **CB-25 (SYS-3.2.1):** An ANR fault shall move the crew box from NORMAL to DEGRADED, and back when the
  fault is cleared.
- **CB-27 (SYS-3.2.7):** The crew box shall toggle the relay-drive output on every tick in NORMAL,
  DEGRADED and MAINT, and hold it inactive in INIT and BYPASS.
  - A stalled processor therefore drops the relay to the analog emergency path.
- **CB-28 (SYS-3.2.7):** The crew box shall enter MAINT only on a CCU command at speed 0.
  - It shall leave MAINT on command, at any speed above 0, or after 10 min.
  - It shall accept the S_hat rotation command only in MAINT.

## 5. Mixer

- **CB-30 (SYS-3.3.1):** The vehicle-warning channel shall have the highest priority.
  - Every lower-priority active channel shall be attenuated by 12 dB while a higher one is active.
- **CB-31 (SYS-3.2.1):** Intercom shall have priority over the two radio channels. The radio channels
  shall have equal priority.
- **CB-32 (SYS-3.2.1):** A channel shall stay active for 300 ms after its signal drops (hold), and stop
  at once when its flag is cleared.
- **CB-33 (SYS-3.3.2):** Sidetone shall be the local microphone at a fixed gain, added without filtering.
- **CB-34 (SYS-3.2.3):** The audio sum shall be peak-limited per frame to 0.5 of full scale (instant
  attack, about 100 ms release).
- **CB-35 (SYS-3.2.3):** The anti-noise shall be added after the limiter, and the output shall saturate
  at full scale without wrap-around.

## 6. Speed-referenced ANR

- **CB-40 (SYS-3.2.2):** The ANR shall generate reference harmonics of f0 = v / track pitch and adapt
  two weights per harmonic by normalised filtered-x LMS.
- **CB-41 (SYS-3.2.2):** With a correct secondary-path estimate, the ANR shall attenuate stationary track
  harmonics by at least 30 dB within 3 s at 50 km/h.
- **CB-42 (SYS-3.2.2):** The ANR shall converge with a secondary-path phase error of up to 60°.
- **CB-43 (SYS-3.2.3):** The anti-noise shall be clamped to 0.5 of full scale.
- **CB-44 (SYS-3.2.2):** The ANR shall be off below 5 km/h or when f0 is below 20 Hz, and shall clear its
  weights when it switches off.
  - It shall not switch on before it has observed the error for 0.2 s.
- **CB-45 (SYS-3.2.2):** Harmonics above 1 kHz shall not be used.
- **CB-46 (SYS-3.2.3):** The ANR guard shall switch the ANR off and report a fault in either case:
  - the error power stays 6 dB above its value at switch-on for 0.2 s, after the first 0.5 s;
  - or a weight exceeds its bound.
- **CB-47 (SYS-3.2.2):** The ANR shall report its attenuation estimate (0.1 dB units) in the uplink
  telemetry.

## Revision history

| Rev | Change | Found by |
|---|---|---|
| A | Lost frames are counted from sequence gaps only. The first version also counted the missing tick, so every corrupted frame counted twice. | Renode IT04 (406 frames lost for 203 CRC errors; now 203) |
| A | The ANR waits 0.2 s of observation before switching on (CB-44). Without it, the guard reference was 0 at the first frame and the guard tripped at once. | host test_anr |
| A | MAINT may also be entered from DEGRADED, so that a faulty ANR can be repaired. | host test_box |
