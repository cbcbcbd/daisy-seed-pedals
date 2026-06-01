# BuzzBox — Notes

> **Validation status:** Control mapping and signal path verified against source (`src/src/buzzbox_hothouse.cpp`). Preset/persistence state verified against working tree + commit history. Re-confirm on hardware before committing any pending work.

Three-mode effect on Hothouse: fuzz drive, envelope-filter auto-wah, and octave (up/down). The committed baseline is Phase 8 (context-dependent control redesign).

## Current control design (verified)
- **Always-active globals:** K1 = Input Gain (0.5×–2.0×), K2 = Dry/Wet Mix, K3 = Output Level.
- **K4–K6 are page-dependent on Toggle 3:**
  - **T3 UP (Fuzz):** K4 = Drive, K5 = Tone Freq (100–1500 Hz), K6 = Gate Threshold
  - **T3 MIDDLE (Autowah):** K4 = Speed (sets ADSR attack/release), K5 = Range, K6 = Threshold
  - **T3 DOWN (Octave):** K4 = Octave-Up Level, K5 = Octave-Down Level, K6 = Octave Mix
- **T1 = autowah placement:** UP = before fuzz, MIDDLE = after fuzz, DOWN = after everything (touch-to-activate).
- **T2 = FS2 effect mode:** UP = autowah + octave, MIDDLE = autowah only, DOWN = octave only (touch-to-activate).
- **T3 = page selector** (does not mark settings dirty, never saved).
- **FS1:** short tap = fuzz on/off; hold 2s (when not in preset mode, FS2 not pressed) = bootloader.
- **FS2:** short tap = autowah/octave on/off per T2 mode.
- **Both FS held 2s:** enter preset mode.
- **LED1:** fuzz state. **LED2:** autowah-or-octave state (different patterns in preset mode).
- All knobs use touch-to-activate; touched K4–K6 flags reset on T3 page change.

## Signal path (verified)
Runs only if an effect is enabled — true bypass (clean passthrough) otherwise.

input → ×input_gain → [Autowah BEFORE, if T1=UP] → [Octave] → [Fuzz] → [Autowah AFTER fuzz, if T1=MIDDLE] → [Autowah AFTER all, if T1=DOWN] → ×2.0 makeup if (autowah|octave) && !fuzz → master lowpass @8kHz → wet/dry mix → ×output_level → out.

- **Autowah detector:** ATone HPF @400Hz → envelope follower → ADSR gate → SVF bandpass (×2).
- **Octave:** decimate /6 → octave generator (up1×level + down1×level) → interpolate → mix.
- **Fuzz:** bass-boost LPF → drive (×1–20) → 4× oversample → aggressive asymmetric clip + harmonics (x²,x³,x⁴) → DC blocker → de-emphasis → gate → tone.

## Open thread — preset / persistence (the testbed pedal)
BuzzBox is where the preset+persistence pattern is being solved first; once working, it ports to the other pedals (Ambien Flux, etc.).

**Important — current state differs from earlier notes.** A prior note recorded this as "reverted to baseline, presets deferred." That revert was never actually applied. The full Phase 9 preset/persistence work is present **uncommitted in the working tree** (committed HEAD is clean Phase 8). It has two distinct parts:

- **Working-state auto-save** — saves settings ~1s after any control movement (debounced `MarkDirty()`), restores on boot via `PersistentStorage<StorageBlock>` on QSPI (magic `0xBEEF0003`). LED1 blinks once on default load, both LEDs blink twice on flash load. *Appears functional; not yet verified on hardware.*
- **4-slot preset mode** — both-FS-2s entry (works, confirmed by LED), tap to navigate slots, hold-both to save, release-both to load, 10s timeout. **Broken:** entry works but the save/load gestures inside preset mode never trigger — LED confirmations never fire, so the gesture conditions are never reached. Root cause unknown.

Supporting changes in the same diff: knob_touched init flipped to `false` (so persisted values aren't blown away by physical knob positions on boot); `toggle_touched[2]` added for T1/T2; `first_start` gate so boot-time toggle reads don't dirty storage; `volatile bool` animation flags driven from the main loop (callback never blocks); FS1 bootloader gate tightened so preset entry can't trigger it.

**Decision pending (do NOT auto-resolve):** whether to commit the working-state-save part, drop the broken preset mode, discard the whole diff, or stash it — pending hardware validation. Leave the working tree as-is until tested.

**Known sub-issues to remember when resuming the broken save/load:**
- Falling edge fires a toggle even after a long press — needs an `ignore_next_release` flag + short-press check.
- Track `both_pressed_start` independently so timing is right when the two switches are pressed at different moments.
- Snapshot settings on preset-mode *entry* so a save stores the original, not a previously loaded preset.
- Next step: research a working Daisy Seed preset implementation before re-attempting.

## Repo notes
- Public/tracked. Committed state = Phase 8 ("global true bypass when both footswitches off").
- Source nesting is unusual: `src/src/buzzbox_hothouse.cpp`. (Structural cleanup candidate — not urgent.)
