# CLAUDE.md — Daisy Seed Pedals

Hothouse guitar pedal projects on the Daisy Seed platform: Funbox ports and original designs.

**Owner:** Chris
**Hardware:** Cleveland Music Co. Hothouse (Daisy Seed-based)
**Working style:** incremental, hardware-validated. Ask before implementing; don't start coding without approval.

<!-- Directory map below is provisional — confirm against actual folders during repo inventory. -->
## Layout
```
daisy-seed-pedals/
├── CLAUDE.md
├── .claude/rules/              # path-scoped reference (DSP, release) — loaded on demand
├── funbox-to-hothouse-ports/   # mars, venus, earth
├── original-hothouse-projects/ # ambien family, buzzbox, flux, simp, tremodulay
├── starter-kit/                # resources for other Hothouse builders
├── tools/                      # checkpoint scripts, utilities
├── libDaisy/  DaisySP/         # pinned submodules
└── docs/
```
Each pedal folder may carry a `NOTES.md` — current architecture + open problems. **When resuming work on any pedal, validate its NOTES.md against the current `.cpp`/`.h` before trusting it.**

## How to work in this repo
- **GUIDED by default:** ask clarifying questions, propose an approach, wait for approval, then implement incrementally. Don't assume requirements.
- **Quote the rule first:** before an action governed by a project rule, quote the relevant rule, then act.
- **Preserve original DSP exactly** when porting — including counterintuitive formulas (e.g. Earth's intentional bloom/fade volume reduction). If something looks wrong, ask before "fixing" it.
- **Never modify** bootloader/DFU entry code or proven parameter curves without explicit instruction.

## Hothouse hardware reference
- **Knobs (6):** `hw.GetKnobValue(Hothouse::KNOB_1)` … `KNOB_6`
- **Toggles (3):** `hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)` — note `TOGGLESWITCH_*`, not `TOGGLE_*`. **Inverted:** physical DOWN=2, MIDDLE=1, UP=0.
- **Footswitches:** `Hothouse::FOOTSWITCH_1` (bypass), `FOOTSWITCH_2` (function)
- **LEDs:** init with `hw.seed.GetPin(Hothouse::LED_1)`; call `led.Update()` after `led.Set()`
- **Controls:** single `hw.ProcessAllControls()` call

## Build
```bash
make clean && make      # build
make program-dfu        # flash via DFU
make program-boot       # install bootloader first (QSPI projects, one-time)
```

## Durable gotchas & patterns (all projects)

### Hardware / platform
- **PersistentStorage.Init():** 2nd param is `address_offset`, not a version. Pass the default struct only.
- **Never call `System::Delay()`** in `ProcessControls()` or the audio callback — blocks audio, causes silence. Use main-loop timing.
- **Knob init** must happen *after* `hw.StartAdc()` with a short settling delay, never before.
- **QSPI bootloader reset:** use `hw.CheckResetToBootloader()`, not manual `System::ResetToBootloader()`.
- **Animations/LEDs:** set a `volatile bool` in the callback, run the animation in the main loop. Never animate in the callback.
- **Seed RNG:** `srand(System::GetNow())` in `main()`, or `rand()` repeats the same sequence every boot.

### DSP
- **Logarithmic knob curve:** `logf(1 + 9*x) / logf(10)` for time-based params — better musical feel than squared (`knob*knob`).
- **Time params:** ~50ms minimum to be usable for delay-type controls.
- **`fonepole()`** takes a `float&` as its first argument (the smoothed value must be `float`, cast to int only when indexing).
- **Zero-crossing detection** with ~1% hysteresis and a 20ms max search window prevents slice-boundary clicks.
- **DaisySP Decimator/Bitcrush is unreliable:** `SetBitsToCrush(0)` gives max crush, not clean. Roll your own sample-and-hold downsampling + low-pass at ~80% of effective Nyquist.
- **Don't process the same DSP object twice per frame** — state corruption (e.g. runaway flanger). Separate instances.
- **`Dust` ships inside `daisysp.h`** — `#include "dust.h"` causes a compile error.
- **Karplus-Strong via DaisySP `String`** is CPU-efficient (delay + filter per voice): ~1170 samples/voice at low E, 6 voices ≈ 28KB, fits SDRAM.

## Common workflows

**New conversion:** read `.claude/rules/dsp-conversion.md` → copy `hothouse.cpp`/`.h` from a working example → get the hardware interface working and validated *first* → then port the DSP, preserving the original exactly.

**Build failures:** check Makefile `TARGET` matches filename → `CPP_SOURCES` includes `hothouse.cpp` → include paths for RTNeural/Eigen if neural → enum names against the Hothouse reference.

**Binary release:** see `.claude/rules/release.md`. <!-- provisional: confirm release reference file name during repo setup -->

## Notes on tooling
Claude Code's auto-memory, checkpoints (rewind), and compaction now handle session continuity, lesson capture, and context — no manual handoff, token-tracking, or snapshot process needed.
