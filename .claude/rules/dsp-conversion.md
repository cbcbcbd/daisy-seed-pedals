---
paths:
  - "**/*.cpp"
  - "**/*.h"
---

# DSP & Conversion Essentials

Path-scoped rule — loads when working on pedal source. Full reference (step-by-step migration, troubleshooting catalog, code samples) lives in `docs/dsp-conversion-full.md`; consult it for detail.

## Proven conversion approach (Funbox → Hothouse)
Order matters. This sequence has worked across Mars, Venus, Earth, BuzzBox:
1. Copy working `hothouse.cpp`/`.h` from an existing project — never recreate.
2. Get the **hardware interface working and validated first** (knobs/toggles/footswitches/LEDs via a simple gain+bypass test) *before* any DSP.
3. Then port the DSP, **preserving the original implementation exactly** — including counterintuitive formulas. If something looks wrong, ask before "fixing" it (e.g. Earth's bloom/fade is an intentional volume reduction).
4. Add complexity incrementally; test each component before integrating.

## Control API (Hothouse)
- `hw.ProcessAllControls()` — single call, replaces Funbox's separate Analog/Digital calls.
- Knobs: `hw.GetKnobValue(Hothouse::KNOB_1)` … `KNOB_6`
- Toggles: `hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)` — **inverted:** DOWN=2, MIDDLE=1, UP=0
- Footswitches: `hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()`
- LEDs: init `led.Init(hw.seed.GetPin(Hothouse::LED_1), false)`; `led.Set()` then `led.Update()`
- Namespace `clevelandmusicco`; object `Hothouse hw;` (not `DaisyPetal`).

## Signal-path topology (get this right or it sounds wrong)
- **Vintage fuzz order:** bass-boost → gain → clip → tone (tone *after* clip = vintage; *before* = modern distortion). Bass-boost before gain = fat clipping; after clip = muddy.
- **Frequency ranges by effect:** fuzz tone 100–1500 Hz; distortion tone 400–3000 Hz; autowah sweep ~200–2500 Hz. Matching range to effect type matters.
- **Tone control type:** simple low-pass for fuzz; tilt-tone (simultaneous HP/LP) for distortion.

## Envelope followers
- Naturally over-sensitive to bass. Add a **pre-emphasis high-pass (300–500 Hz) on the detection signal only** — leave the audio path full-range — for even triggering across the fretboard. Lower the threshold to compensate for reduced detection level.
- `ATone::SetFreq()` needs a variable reference, not a literal (`float f = 400.f; hpf.SetFreq(f);`).

## Gate
- Simple internal gate usually best: detect on the **gained** signal (after bass-boost/gain), apply to the clipped signal, threshold calibrated to that gained level. Don't overcomplicate with separate detect/apply paths unless needed.

## Multirate
- Declare cross-rate buffer variables outside the callback, init to 0, update only when processing, let the value persist between updates. Buffer alignment is critical — indices increment *after* use, not before.

## Stateful-object hazard
- Don't process the same stateful DSP object (Tone, filters) twice in one audio cycle — accumulates state, causes squealing/oscillation. Use separate instances or process once per cycle. (Also in CLAUDE.md globals.)

## UI consistency
- In context-dependent control layouts (e.g. T3 page selector), keep similar functions on the same knob across pages (gate/threshold always on K6, etc.) for muscle memory.
- Set sensible defaults so the pedal sounds usable immediately at noon settings.

## Makefile / build
- `CPP_SOURCES` must include `hothouse.cpp`. `USE_DAISYSP_LGPL = 1`. `OPT = -Ofast`. `APP_TYPE = BOOT_QSPI` for complex projects (needs bootloader: `make program-boot` once, then `make program-dfu`).
- Build-fail checklist: TARGET matches filename → `hothouse.cpp` in CPP_SOURCES → RTNeural/Eigen include paths if neural → toggle/enum names match the Hothouse reference.
