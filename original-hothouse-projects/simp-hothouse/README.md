# Simp - Polyphonic Sub-Octave Synthesizer

**Status:** Research/Planning Phase  
**Platform:** Hothouse  
**Inspiration:** Chase Bliss Lost & Found "Impulse Synthesizer"

## Overview

Simp is a sub-octave guitar synthesizer using Karplus-Strong string synthesis triggered by pitch detection. Creates rich, cinematic string textures from guitar input.

## Current Status

**ON HOLD** - Awaiting external consultation on approach:
- Monophonic pitch detection + rich voicing (fast, proven)
- Polyphonic 4-voice detection (ambitious, research-heavy)
- Staged development (v1.0 mono, v2.0 poly)

## Architecture (Planned)

```
Guitar Input → Pitch Detection → Sub-Octave Math → 4x StringVoice → Mix → Output
```

### Pitch Detection Options
- **Q DSP Library** (monophonic, proven on Daisy via BK Shepherd)
- **Custom Polyphonic** (requires R&D, 2-4 months)

### String Synthesis
- Using `daisysp::StringVoice` (extended Karplus-Strong)
- 4 voices: Root, -1 oct, -2 oct, +5th or variations
- Rich parameter control: brightness, damping, structure, accent

## Hardware Requirements

- Cleveland Music Co. Hothouse
- Daisy Seed
- Guitar input

## Build Instructions

```bash
# Build firmware
make

# Flash to Daisy
make program-dfu
```

## Control Mapping (Proposed)

### Knobs
- K1: Dry/Wet Mix
- K2: Sub-1 Octave Level
- K3: Sub-2 Octave Level  
- K4: Brightness
- K5: Decay
- K6: Structure

### Toggles
- T1: Sustain Mode (decay vs continuous)
- T2: Tracking Speed (fast vs portamento)
- T3: Voicing Mode (octaves / fifth / detune)

### Footswitches
- FS1: Bypass
- FS2: Freeze (hold/layer notes)

### LEDs
- LED1: Effect Status
- LED2: Pitch Detection Active

## Development Phases

### Phase 0: Research (CURRENT)
- ✅ Gathered Chase Bliss hardware intel
- ✅ Analyzed DaisySP options
- ✅ Found Q DSP library + BK Shepherd reference
- ⏳ Awaiting external guidance on approach

### Phase 1: Proof of Concept (When Approved)
- Integrate Q DSP pitch detection
- Test with guitar input
- Validate CPU usage
- Confirm latency acceptable

### Phase 2: Single Voice Prototype
- Connect pitch detector to StringVoice
- Implement triggering
- Basic dry/wet control

### Phase 3: Multi-Voice Expansion
- Add 3 more StringVoice instances
- Sub-octave calculation
- Voice mixing and balance

### Phase 4: Hothouse Integration
- Complete control mapping
- Toggle modes
- LED feedback
- Testing and polish

## References

- **Q DSP Library:** https://github.com/cycfi/q
- **BK Shepherd Tuner:** Reference implementation for Q DSP on Daisy
- **Chase Bliss Lost + Found:** Inspiration (4-voice polyphonic, Cortex-M7)
- **DaisySP StringVoice:** Extended Karplus-Strong from Mutable Rings

## Notes

See `docs/simp_project_notes.md` for complete research and technical analysis.

## License

TBD

---

**Last Updated:** 2025-11-18
