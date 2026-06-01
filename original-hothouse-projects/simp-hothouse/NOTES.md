# Simp — Notes

> **Status: concept / skeleton only.** No working implementation yet. Committed as "Simp project skeleton — awaiting development direction." This file captures the design thinking so it isn't lost; validate nothing against it until real code exists.

Concept: a **polyphonic guitar synthesizer** for Hothouse. Detect the pitch of what's played and resynthesize it with Karplus-Strong string synthesis, with controls for glide, harmonic texture, and envelope shaping.

## The two technical problems
1. **Pitch detection (the hard, unsolved one).** DaisySP has no built-in pitch detection — this needs a custom implementation, and it's the primary blocker. This is the part to research / get outside input on before committing to an approach.
2. **String synthesis (tractable).** DaisySP's `String` class does Karplus-Strong, which makes the synthesis side straightforward once pitch is known. It's CPU-efficient (just delay + filter per voice), so 4–6 voice polyphony is feasible: ~1170 samples/voice at low E (41 Hz), 6 voices ≈ 28KB — fits in SDRAM. (This is also captured as a global gotcha in CLAUDE.md.)

## Proposed control concept (not yet built)
- **Portamento** — pitch glide, likely `fonepole` smoothing on the delay-line length.
- **Texture** — harmonic complexity / detuning.
- **Attack/Release** — envelope shaping, likely an envelope filter with a gate.

## Open question before development resumes
Pitch detection approach is undecided and is the gating item. Worth noting Chris was uncertain this ever worked correctly — it most likely never reached working code, stalling at the design stage on the pitch-detection question. Revisit with that as the first problem to solve.
