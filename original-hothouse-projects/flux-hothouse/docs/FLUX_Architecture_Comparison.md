# FLUX: Two Architectures Compared

## Visual Comparison: Tremolo Slicer vs Sample & Hold

---

## ARCHITECTURE 1: TREMOLO SLICER (Phase 2 - Current)

```
┌─────────────────────────────────────────────────────────────────┐
│                    CONTINUOUS FLOW PARADIGM                      │
└─────────────────────────────────────────────────────────────────┘

INPUT (Guitar)
  │
  │ [Always flowing]
  ▼
┌─────────────────┐
│  DELAY BUFFER   │  ◄── Continuous write, always recording
│   (2 seconds)   │
│  [::::::::::::]  │
└─────────────────┘
  │
  │ [Read from TIME position]
  ▼
┌─────────────────┐
│     PHASOR      │  ◄── 0.0 → 1.0 ramp (SLICE RATE)
│   [/|/|/|/|]    │
└─────────────────┘
  │
  │ [Generate gate pattern]
  ▼
┌─────────────────┐
│ PATTERN LOGIC   │  ◄── Quarter/Eighth/Sixteenth
│  1/4: ████░░░░  │
│  1/8: ██░░██░░  │
│ 1/16: █░█░█░█░  │
└─────────────────┘
  │
  │ [Gate = 0.0 or 1.0]
  ▼
┌─────────────────┐
│  GATE SMOOTHER  │  ◄── Fonepole to prevent clicks
│   [smooth ~~~]   │
└─────────────────┘
  │
  │ [Multiply delay by gate]
  ▼
┌─────────────────┐
│   APPLY GATE    │  ◄── delayOut *= (1.0 - depth*(1-gate))
│  [SLICE DEPTH]  │
└─────────────────┘
  │
  ├────────────────► FEEDBACK PATH (regenerates)
  │
  ▼
  MIX with DRY → OUTPUT

FEEL: Musical, rhythmic, like tremolo on delay
CPU: ~15% (very efficient)
MEMORY: ~400KB
```

---

## ARCHITECTURE 2: SAMPLE & HOLD (Proposed FLUX)

```
┌─────────────────────────────────────────────────────────────────┐
│                   DISCRETE CAPTURE PARADIGM                      │
└─────────────────────────────────────────────────────────────────┘

INPUT (Guitar)
  │
  │ [Capture discrete moments]
  ▼
┌────────────────────────────────────────────────────────────────┐
│              SLICE CAPTURE ENGINE                              │
│                                                                │
│  Slice 1: [■■■■■■] (50-500ms)  ◄── Individual buffers         │
│  Slice 2: [■■■■■■■] (random length)                           │
│  Slice 3: [■■■■] (variability)                                 │
│  Slice 4: [■■■■■■■■] (each independent)                        │
│  ...                                                           │
│  Slice N: [■■■■■] (up to 16 slices)                           │
└────────────────────────────────────────────────────────────────┘
  │
  │ [Discrete moments, frozen in time]
  ▼
┌────────────────────────────────────────────────────────────────┐
│                  PLAYBACK SEQUENCER                            │
│                                                                │
│  • Which slice to play? (Forward/Reverse/Random)               │
│  • How many repeats? (Stutter amount)                          │
│  • What order? (Sequential/Random/Pattern)                     │
│                                                                │
│  Current: Slice 3                                              │
│  Repeat: 2 of 4 ──► [Play] [Play] [Play] [Play] → Next       │
└────────────────────────────────────────────────────────────────┘
  │
  │ [Each slice can be processed differently]
  ▼
┌────────────────────────────────────────────────────────────────┐
│              PER-SLICE PROCESSING                              │
│                                                                │
│  Slice 1: Clean      [====]                                    │
│  Slice 2: Reverse    [====>]                                   │
│  Slice 3: Shimmer    [↑===]                                    │
│  Slice 4: Granular   [≈≈≈≈]                                    │
└────────────────────────────────────────────────────────────────┘
  │
  ├────────────────► FEEDBACK (re-slice processed audio)
  │
  ▼
  MIX with DRY → OUTPUT

FEEL: Glitchy, stuttering, like MPC/sampler
CPU: ~40-50% (more processing)
MEMORY: ~1.5MB (multiple buffers)
```

---

## KEY DIFFERENCES TABLE

| Aspect | Tremolo Slicer | Sample & Hold |
|--------|----------------|---------------|
| **Audio Flow** | Continuous stream | Discrete moments |
| **Buffer Type** | Single delay line | Multiple slice buffers |
| **Pattern Source** | Mathematical (phasor) | Captured samples |
| **Time Feel** | Fluid, flowing | Frozen, stuttering |
| **Repetition** | Via feedback | Via slice repeat count |
| **Variability** | Pattern selection | Random lengths/order |
| **Processing** | Applied to whole signal | Per-slice different FX |
| **Predictability** | High (musical) | Low (experimental) |
| **CPU Usage** | Low (15%) | Medium (40-50%) |
| **Memory** | Low (400KB) | Medium (1.5MB) |
| **Complexity** | Simple | Complex |
| **Best For** | Musical delays | Glitch/experimental |

---

## CONTROL COMPARISON

### TREMOLO SLICER (Current Phase 2)

```
K1: TIME          → Delay time (where to read)
K2: MIX           → Dry/wet blend
K3: FEEDBACK      → Regeneration amount
K4: SLICE DEPTH   → Gate intensity (0=no chop, 100=full chop)
K5: SLICE RATE    → Pattern speed (phasor frequency)
K6: (unused)      → Reserved for modulation

Toggle 1: Quarter / Eighth / Sixteenth patterns
Toggle 2: (unused) Reserved for pattern tables
Toggle 3: (unused) Reserved for tap tempo
```

### SAMPLE & HOLD (Proposed)

```
K1: TIME          → Pre-delay before slicing
K2: MIX           → Dry/wet blend
K3: FEEDBACK      → Pattern regeneration
K4: SLICE COUNT   → How many samples (1-16)
K5: SLICE LENGTH  → Duration of each sample (10-500ms)
K6: STUTTER       → Repetition probability/intensity

Toggle 1: Capture/Playback mode (Forward/Reverse/Random)
Toggle 2: Per-slice effect (Clean/Reverse/Shimmer)
Toggle 3: Shift modes (Normal/Mod/Lo-Fi)
```

---

## SOUND CHARACTER COMPARISON

### TREMOLO SLICER Examples

**Quarter Notes, Low Depth (30%), Slow Rate (1Hz):**
```
Audio: ████████░░░░░░░░████████░░░░░░░░████████
Feel: Gentle rhythmic pulsing, like slow tremolo
Use: Ambient pads, swelling delays
```

**Sixteenth Notes, High Depth (90%), Fast Rate (8Hz):**
```
Audio: █░█░█░█░█░█░█░█░█░█░█░█░█░█░█░
Feel: Aggressive stutter, almost ring mod
Use: Glitch effects, rhythmic chaos
```

**Eighth Notes, Med Depth (60%), Tempo (120 BPM):**
```
Audio: ████░░░░████░░░░████░░░░████░░░░
Feel: Syncopated delay, locked to tempo
Use: Rhythmic solos, dance music
```

---

### SAMPLE & HOLD Examples

**4 Slices, 100ms each, 3 repeats, Forward:**
```
Capture: [A][B][C][D]
Output:  [A][A][A][B][B][B][C][C][C][D][D][D]
Feel: Deliberate stutter, like MPC repeat
Use: Hip-hop style repeats, stutter edits
```

**8 Slices, Random Length (50-200ms), 1 repeat, Random Order:**
```
Capture: [A:80ms][B:150ms][C:60ms][D:200ms]...[H:100ms]
Output:  [F][C][A][H][D][B][E][G]... (shuffled)
Feel: Chaotic granular texture
Use: Experimental soundscapes, glitch art
```

**3 Slices, 250ms each, Variability 80%, Reverse Play:**
```
Capture: [A:350ms][B:180ms][C:290ms] (lengths vary)
Output:  [C][B][A][C][B][A][C][B][A]...
Feel: Evolving reverse texture
Use: Ambient drones, reverse delays
```

---

## USE CASE SCENARIOS

### When to Use TREMOLO SLICER

✅ **Rhythmic delay for solos** - Lock to tempo, create syncopation
✅ **Ambient pads with movement** - Gentle pulsing under sustained chords
✅ **Dance/electronic music** - Tempo-locked rhythmic delays
✅ **Predictable performance** - Know what you're getting
✅ **Low CPU scenarios** - Need headroom for other effects
✅ **Traditional pedal users** - Familiar delay + tremolo concept

### When to Use SAMPLE & HOLD

✅ **Experimental soundscapes** - Unpredictable, evolving textures
✅ **Glitch/IDM production** - Stutters, repeats, granular chaos
✅ **Sound design work** - Creating unique, one-off sounds
✅ **Compositional tool** - Build tracks from captured moments
✅ **Live improvisation** - React to what pedal captures
✅ **Avant-garde performance** - Embrace the chaos

---

## PRODUCT POSITIONING

### TREMOLO SLICER
- **Name:** "Rhythmic Delay" or "Pulse Delay"
- **Price:** $200-250
- **Market:** Indie/ambient guitarists, electronic musicians
- **Comparison:** Boss SL-2 + delay in one pedal
- **Tagline:** "Rhythmic delays that lock to your tempo"

### SAMPLE & HOLD
- **Name:** "FLUX - Modulated Slicer Delay"
- **Price:** $300-350
- **Market:** Experimental musicians, sound designers
- **Comparison:** Chase Bliss territory
- **Tagline:** "Turn your guitar into a glitch machine"

---

## DEVELOPMENT TIMELINE

### TREMOLO SLICER
- Phase 1: ✅ Complete
- Phase 2: ✅ Complete
- Phase 3: Add tap tempo (1 week)
- Phase 4: Add pattern library (2 weeks)
- Phase 5: Polish & presets (1 week)
- **Total:** ~1 month from current state

### SAMPLE & HOLD
- Phase 1: Architecture redesign (1 week)
- Phase 2: Slice capture engine (2 weeks)
- Phase 3: Playback sequencer (2 weeks)
- Phase 4: Per-slice processing (3 weeks)
- Phase 5: Shift modes (2 weeks)
- Phase 6: Polish & optimization (2 weeks)
- **Total:** ~3 months from scratch

---

## RECOMMENDATION

### Keep Both!

**Option 1: Two Separate Products**
- "Pulse Delay" (Tremolo Slicer) - $200-250
- "FLUX" (Sample & Hold) - $300-350

**Option 2: Multi-Mode FLUX**
- Mode 1: Tremolo Slicer (Toggle 3 UP)
- Mode 2: Sample & Hold (Toggle 3 MIDDLE)
- Mode 3: Freeze/Sustain (Toggle 3 DOWN)
- Price: $350-400

**Option 3: Platform Approach**
- Core platform with swappable "brains"
- Different firmware images
- Let users choose their flavor

---

**The Tremolo Slicer work is complete and valuable. Don't throw it away - it's a different product solving a different need.**

---

END OF COMPARISON DOCUMENT
