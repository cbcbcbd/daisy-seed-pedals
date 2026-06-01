# Ambien Flux — Notes

> **Validation status:** Control mapping and signal path verified against source (`ambien_flux.cpp`) and v1.0 release commits.

Released sample-and-hold slicer delay on Hothouse. **Public, shipped v1.0.** Targets the experimental/glitch space. (Internal versioning called this "FLUX v3.x"; it shipped as Ambien Flux v1.0 — same pedal, renamed at release. The folder was renamed `flux-hothouse → ambien-flux-hothouse`.)

Architecture: discrete sample-and-hold — independent slice buffers, each variable length, captured and played back. Full spec: `FLUX_v3_1_SampleAndHold_Specifications.md`.

## Control mapping (verified, as shipped)
- **K1** = Master Level (0–200%, unity→+6 dB), always active
- **K2** = Dry/Wet Mix, always active
- **K3–K6** = page-dependent on Toggle 3 (touch-to-activate; flags reset on T3 change)
  - **T3 UP (Normal):** K3 = Feedback (pattern regen), K4 = Slice Count (1–16), K5 = Slice Length (100–500 ms, log), K6 = Stutter (shuffle probability)
  - **T3 DOWN (Lo-Fi):** K3 = Wobble (LFO depth/rate), K4 = Dust (density + mix), K5 = Bit Crush (S&H amount), K6 = unused
  - **T3 MIDDLE:** reserved (intended for the envelope system — see below)
- **T1 = capture/playback mode:** UP = fwd/fwd, MIDDLE = back/reverse, DOWN = fwd capture + random direction per slice
- **T2:** unused — reserved for the envelope system (code commented out, class definition + uncomment markers preserved)
- **FS1:** tap = bypass; hold 2s = bootloader
- **FS2:** tap = freeze (latching — stops capture, keeps playing the current buffer)
- **LED1:** effect-on. **LED2:** freeze-on.

## Signal path (verified)
input → CustomBitCrush (S&H + LP @50% Nyquist) → +(wet×feedback) → CaptureSlice (ring of 16 slices) → PlaybackSlice (zero-crossing detection, 15% variable crossfade, stutter repeats) → crossfade against dry by mix → Wobble (LFO delay, only if wobble>0 & mix>0) → Dust (sparse impulses, only if noise>0 & mix>0) → ×master_level → out L/R.

- Bypass = clean passthrough.
- Memory: 16 slices × 24000 samples (500 ms max) in SDRAM.

## Status / open threads
- **Persistence/presets: not started here.** No PersistentStorage, no save/load — settings reset every power cycle. The preset+persistence pattern is being solved on **BuzzBox first** (see `buzzbox-hothouse/NOTES.md`), then ported here. T2 and T3-MIDDLE are reserved and the envelope system is built-but-commented — both are the natural next expansion once persistence lands.
- **Tech debt note:** this started as personal-use code before the cleaner shared-project patterns were adopted, so some areas predate current conventions. Worth a tidy pass if it becomes a public reference for other builders.

## Licensing
- Uses Neptune delayline (MIT) — attribution required in releases (present in v1.0).
