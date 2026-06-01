# Ambien — Notes

> **Validation status:** Control mapping and signal path verified against source (`ambien_main.cpp`, 693 lines) and commit history. Current code is **v1.2** despite a stale "Ambien Delay v1.0" header in the file (line 8). Canonical name: **Ambien** (folder `ambien-hothouse`); "Ambien Delay" is the legacy in-file name.

Spectral phasing + granular delay on Hothouse. **Public/tracked.** This is the parent that **Ambien Flux** split off from — Ambien is the *textured/phased* sibling (slices carry a flanged sound); Ambien Flux is the *glitch/lo-fi* sibling. See `ambien-flux-hothouse/NOTES.md`.

Architecture (v1.2): 3-band spectral flanger → flanged signal captured into slice buffers → granular playback with per-slice decay. Flanging is "baked into" the slices.

## Control mapping (verified)
All 6 knobs are page-dependent on Toggle 3 — there are **no global knobs**. Note the band-paired layout on Page 2 (K1+K4 = low, K2+K5 = mid, K3+K6 = high).

| | **T3 UP — Core Delay** | **T3 MIDDLE — Band Vol / Q** | **T3 DOWN — Flanger** |
|---|---|---|---|
| **K1** | Master Level (0–200%, unity@50%) | Low Volume (0–200%, unity@50%) | Low Flanger Depth (0–100%) |
| **K2** | Dry/Wet Mix (0–100%) | Mid Volume | Mid Flanger Depth |
| **K3** | Feedback → *slice decay rate* (see note) | High Volume | High Flanger Depth |
| **K4** | Slice Count (3–16, see WIP note) | Low Q (0.1–2.0) | Low Flanger Rate (0.05–10 Hz, exp) |
| **K5** | Slice Length (100–500 ms, log) | Mid Q (0.1–2.0, ×0.7 internal) | Mid Flanger Rate |
| **K6** | Crossfade Length (0–50% of slice, 20 ms min) | High Q (0.1–2.0, ×0.5 internal) | High Flanger Rate |

- **T1 = playback direction:** UP = forward, MIDDLE = reverse, DOWN = random per slice
- **T2:** unused (reserved)
- **T3:** page selector
- **FS1:** slicer on/off (latching toggle). **FS2:** spectral flanger on/off (latching toggle).
- **LED1:** slicer state. **LED2:** flanger state.
- **Boot:** both effects OFF (true bypass on power-up).
- **Bootloader:** `hw.CheckResetToBootloader()` in main loop (QSPI-compatible — note this differs from Ambien Flux, which uses manual `System::ResetToBootloader()`).
- All 6 knobs use touch-to-activate; page change resets touch flags and re-snapshots positions.

## Signal path (verified)
input splits to dry + processed:
- **Spectral flanger (FS2):** 3-band SVF split — Low `SVF.Low()` @800Hz, Mid `SVF.Band()` @900Hz, High `SVF.High()` @1000Hz — each into its own flanger (feedback 0.85, per-band depth/rate), summed with per-band volumes → `flangedSignal`.
- **Slicer (FS1):** captures `flangedSignal` (flanging baked in; zero-cross detect, 50ms/2400-sample search window, ring of 16 slice buffers) → playback with equal-power √ crossfade and per-slice volume decay.
- **Mix:** `wet = slicer ? sliced : flangedSignal`. Both off → true bypass (`out = input`). Else equal-power: `out = input·√(1−mix) + wet·√(mix)` → ×master_level → out L/R.
- Slice memory: 16 × 24000 samples (500ms) in SDRAM. Block size 512.

## Things a future reader must know (don't "fix" these)
- **"Feedback" (K3) is not delay feedback** — it's a per-slice volume decay: `decay_factor = 0.5 + 0.45 × fb`, applied each time a slice replays. Players coming from delay pedals will misread it.
- **Per-band Q scaling is intentional** (frequency compensation): low ×1.0, mid ×0.7, high ×0.5 internally, even though the K4–K6 knobs all report 0.1–2.0. Deliberate — don't normalize.
- **Zero-cross search window = 2400 samples (50ms)** was deliberately tuned. Longer = cleaner zero-crossing but worse phase alignment / more clicks. Don't change without an ear test.
- **v1.1 fixed infinite repeats** by giving the original signal separate filter/flanger instances — processing one DSP object twice per frame corrupts state (see the global gotcha in CLAUDE.md).

## Status / open threads
- **Uncommitted WIP:** K4 slice-count range changed 1–16 → 3–16 (`(int)(knob*13.999f)+3`), a standalone hand-test fix — single-slice playback likely clicked/locked (relates to read/write conflict at low slice counts). **Unverified on hardware; do not commit until tested.** If it checks out, safe to commit as a v1.2.1 hotfix.
- **No README** in this folder (one of two without one). A README is worth adding if Ambien moves toward release.
- **Persistence/presets:** not started (same as Ambien Flux) — pattern to be solved on BuzzBox first, then ported.

## Commit history
v1.0 (granular delay, local only) → v1.1 (separate filters/flangers, fixes infinite repeats) → v1.2 (independent FS controls, auto-passthrough, filter-flange-capture architecture). Current.
