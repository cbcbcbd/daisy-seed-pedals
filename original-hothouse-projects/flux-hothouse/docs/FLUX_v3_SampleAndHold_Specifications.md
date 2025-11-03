# FLUX v3.0: Sample & Hold Slicer Delay
## Complete Specifications - Sample & Hold Architecture

**Date:** 2025-10-26  
**Version:** 3.0 - Complete architectural redesign  
**Previous:** v2.0 (Tremolo Slicer - see snapshot document)

---

## DOCUMENT HISTORY

- **v1.0 (2025-10-15):** Initial tremolo slicer concept
- **v2.0 (2025-10-16):** Refined tremolo with shift modes
- **v3.0 (2025-10-26):** Complete pivot to Sample & Hold architecture
  - Discrete buffer capture instead of continuous flow
  - Slice count, length, and stutter controls
  - Three-mode shift system (Normal/Mod/Lo-Fi)
  - Per-slice processing architecture
  - Inspired by MPC/sampler paradigm

---

## 1. OVERVIEW

**Concept:** A Sample & Hold delay pedal that captures discrete audio slices, stores them in individual buffers, and plays them back with controllable repetition, randomization, and per-slice processing. Think "MPC meets Chase Bliss" - the deliberate stutter and glitch of a sampler with the experimental chaos of a boutique delay.

**Philosophy:** Transform guitar into a glitch machine by freezing discrete moments in time and manipulating them rhythmically. Instead of continuous delay with gating (tremolo style), we capture distinct audio snapshots that can be repeated, reordered, and processed independently.

**Target Use Cases:**
- Experimental/glitch guitarists
- IDM/electronic music production  
- Sound design and composition
- Live improvisation with captured moments
- Ambient textures from discrete slices
- Stuttering/repeat effects

---

## 2. CORE ARCHITECTURE

### Sample & Hold Paradigm

**NOT a continuous delay line with gating.**  
**IS a discrete buffer capture and playback system.**

```
INPUT → CAPTURE ENGINE → SLICE BUFFERS → PLAYBACK SEQUENCER → OUTPUT
              ↓              ↓                  ↓
         (Records)     (Stores)           (Plays back)
```

### Key Conceptual Shift

**Old (Tremolo):** Audio flows continuously, gate turns it on/off rhythmically  
**New (S&H):** Audio is captured in discrete chunks, chunks are frozen and manipulated

---

## 3. CONTROL MAPPING

### 3.1 Normal Mode (Toggle 3 UP)

#### Knobs - Core Slicing Parameters

| Knob | Parameter | Range | Function | Notes |
|------|-----------|-------|----------|-------|
| **K1** | TIME | 50ms - 2s | Pre-delay before slicing | Optional delay before capture |
| **K2** | MIX | 0% - 100% | Dry/wet blend | Constant power crossfade |
| **K3** | FEEDBACK | 0% - 120% | Pattern regeneration | Sliced audio feeds back |
| **K4** | SLICE COUNT | 1 - 16 | Number of slices per cycle | More = more granular |
| **K5** | SLICE LENGTH | 10ms - 500ms | Duration of each slice | Shorter = glitchier |
| **K6** | STUTTER | 0% - 100% | Repetition probability | 0=no repeat, 100=heavy glitch |

#### Toggle Switches

**Toggle 1: Capture/Playback Mode**
- **UP:** Forward/Forward (sequential capture, sequential play)
- **MIDDLE:** Forward/Reverse (sequential capture, reverse playback)
- **DOWN:** Random/Random (random capture, random playback)

**Toggle 2: Per-Slice Effect**
- **UP:** Clean (no processing)
- **MIDDLE:** Reverse (backwards playback per slice)
- **DOWN:** Shimmer (octave up per slice)

**Toggle 3: Mode Selector**
- **UP:** Normal Mode (core slicing controls)
- **MIDDLE:** Shift Mode A (modulation & tone)
- **DOWN:** Shift Mode B (lo-fi & output)

#### Footswitches

**FS1 (Left):**
- **Tap:** Toggle bypass
- **Hold 600ms:** (Reserved for future)
- **Hold 2s:** Bootloader entry

**FS2 (Right) - Context Sensitive:**
- **Normal Mode:** FREEZE (capture and loop current pattern)
- **LED2:** Shows freeze state (solid when frozen)

---

### 3.2 Shift Mode A (Toggle 3 MIDDLE) - Modulation & Tone

#### Knobs - Modulation Parameters

| Knob | Parameter | Range | Function | Notes |
|------|-----------|-------|----------|-------|
| **K1** | MOD RATE | 0.05Hz - 10Hz | LFO speed | Slow drift to fast wobble |
| **K2** | MOD DEPTH | 0% - 100% | LFO intensity | How much modulation |
| **K3** | MOD TARGET | 0% - 100% | What gets modulated | See below |
| **K4** | TONE | 0% - 100% | Filter cutoff | Low-pass on slices |
| **K5** | DRIVE | 0% - 100% | Saturation amount | Distortion/overdrive |
| **K6** | DIFFUSION | 0% - 100% | Texture/smear | Adds "cloud" quality |

#### K3 MOD TARGET Behavior

Knob position determines modulation destination:
- **0-33%:** Modulate SLICE LENGTH (vibrato-style warping)
- **34-66%:** Modulate FILTER CUTOFF (sweeping tone)
- **67-100%:** Modulate SLICE COUNT (rhythmic density variation)

#### Toggle Behavior
- **Toggle 1 & 2:** Same as Normal Mode
- **Toggle 3:** MIDDLE position activates this mode

#### Footswitch Behavior
**FS2:** TAP TEMPO for modulation LFO
- **LED2:** Blinks at LFO rate

---

### 3.3 Shift Mode B (Toggle 3 DOWN) - Lo-Fi & Output

#### Knobs - Lo-Fi Parameters

| Knob | Parameter | Range | Function | Notes |
|------|-----------|-------|----------|-------|
| **K1** | MASTER LEVEL | -12dB to +6dB | Overall output volume | Compensate for buildup |
| **K2** | BIT CRUSH | 0% - 100% | Sample rate reduction | Digital degradation |
| **K3** | WOBBLE | 0% - 100% | Pitch instability | Tape wow/flutter |
| **K4** | NOISE | 0% - 100% | Background hiss | Analog tape character |
| **K5** | SPREAD | 0% - 100% | Stereo width | If stereo build |
| **K6** | AGE | 0% - 100% | Master lo-fi amount | Controls 2,3,4 together |

#### K6 AGE Master Control

When turned up, simultaneously increases:
- Bit crush amount
- Wobble intensity  
- Noise level
- Creates unified "degraded tape" effect

#### Toggle Behavior
- **Toggle 1 & 2:** Same as Normal Mode
- **Toggle 3:** DOWN position activates this mode

#### Footswitch Behavior
**FS2:** DRY KILL toggle
- Mutes dry signal, wet only
- **LED2:** Shows dry kill state (solid when dry killed)

---

## 4. SIGNAL FLOW ARCHITECTURE

### 4.1 Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                  INPUT (Guitar/Instrument)                       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                 ┌───────────▼───────────┐
                 │    PRE-DELAY (K1)     │  Optional delay before capture
                 │   (50ms - 2 seconds)  │
                 └───────────┬───────────┘
                             │
         ┌───────────────────▼───────────────────┐
         │       SLICE CAPTURE ENGINE            │
         │                                       │
         │  • Captures K4 slices (1-16)          │
         │  • Each slice K5 length (10-500ms)    │
         │  • Mode determined by Toggle 1        │
         │    - Sequential capture               │
         │    - Random capture                   │
         │    - Reverse capture                  │
         └───────────────────┬───────────────────┘
                             │
         ┌───────────────────▼───────────────────┐
         │         SLICE BUFFER ARRAY            │
         │                                       │
         │  Buffer[0]: [■■■■■■■] ← Slice 1      │
         │  Buffer[1]: [■■■■■■]  ← Slice 2      │
         │  Buffer[2]: [■■■■■■■■] ← Slice 3     │
         │  ...                                  │
         │  Buffer[N]: [■■■■■]   ← Slice N      │
         │                                       │
         │  • Individual buffers per slice       │
         │  • Variable lengths (randomized)      │
         │  • Independent storage                │
         └───────────────────┬───────────────────┘
                             │
         ┌───────────────────▼───────────────────┐
         │      PLAYBACK SEQUENCER               │
         │                                       │
         │  • Determines playback order          │
         │  • Applies stutter (K6)               │
         │  • Handles repetition count           │
         │                                       │
         │  Current Slice: 3                     │
         │  Repeat: 2 of 4                       │
         │  Next: Slice 4                        │
         └───────────────────┬───────────────────┘
                             │
         ┌───────────────────▼───────────────────┐
         │     PER-SLICE PROCESSING              │
         │       (Toggle 2 selection)            │
         │                                       │
         │  ┌─────────────────────────────────┐  │
         │  │ Clean: Pass through             │  │
         │  │ Reverse: Play backwards         │  │
         │  │ Shimmer: Octave up + original   │  │
         │  └─────────────────────────────────┘  │
         └───────────────────┬───────────────────┘
                             │
         ┌───────────────────▼───────────────────┐
         │    SHIFT MODE PROCESSING              │
         │     (Toggle 3 determines)             │
         │                                       │
         │  Mode A: Modulation + Tone            │
         │    • LFO modulation (K1,K2,K3)        │
         │    • Filter (K4)                      │
         │    • Drive (K5)                       │
         │    • Diffusion (K6)                   │
         │                                       │
         │  Mode B: Lo-Fi + Output               │
         │    • Bit crush (K2)                   │
         │    • Wobble (K3)                      │
         │    • Noise (K4)                       │
         │    • Master level (K1)                │
         └───────────────────┬───────────────────┘
                             │
                 ┌───────────▼───────────┐
                 │   FEEDBACK PATH (K3)  │
                 │                       │
                 └───────────┬───────────┘
                             │
                         ┌───┴────┐
                         │        │
                     Feedback   Wet Out
                         │        │
                         └────┬───┘
                              │
                 ┌────────────▼────────────┐
                 │  DRY/WET MIX (K2)       │
                 │  Constant Power Xfade   │
                 └────────────┬────────────┘
                              │
                 ┌────────────▼────────────┐
                 │     OUTPUT (Pedal)      │
                 └─────────────────────────┘
```

---

## 5. DETAILED FEATURE DESCRIPTIONS

### 5.1 Slice Capture Engine

**Purpose:** Records incoming audio into discrete buffer slices

**Parameters:**
- **SLICE COUNT (K4):** How many slices to capture (1-16)
- **SLICE LENGTH (K5):** Duration of each slice (10-500ms)  
- **Capture Mode (Toggle 1):** How to capture (sequential/random/reverse)

**Operation:**

```cpp
// Pseudo-code for capture
void CaptureSlice() {
    int currentSlice = captureIndex;
    int sliceLength = CalculateSliceLength(K5, variability);
    
    // Record audio into slice buffer
    for (int i = 0; i < sliceLength; i++) {
        sliceBuffers[currentSlice][i] = input[i];
    }
    
    // Advance to next slice based on mode
    if (captureMode == SEQUENTIAL) {
        captureIndex = (captureIndex + 1) % sliceCount;
    } else if (captureMode == RANDOM) {
        captureIndex = random(0, sliceCount);
    } else if (captureMode == REVERSE) {
        captureIndex = (captureIndex - 1 + sliceCount) % sliceCount;
    }
}
```

**Capture Modes (Toggle 1):**

**UP - Sequential Capture:**
```
Slice order: 1 → 2 → 3 → 4 → 1 → 2 → 3 → 4...
Predictable, orderly
```

**MIDDLE - Reverse Capture:**
```
Slice order: 4 → 3 → 2 → 1 → 4 → 3 → 2 → 1...
Backwards flow
```

**DOWN - Random Capture:**
```
Slice order: 3 → 1 → 4 → 2 → 1 → 3 → 4 → 2...
Unpredictable, chaotic
```

---

### 5.2 Slice Buffer Array

**Structure:**
```cpp
#define MAX_SLICES 16
#define MAX_SLICE_LENGTH 24000  // 500ms @ 48kHz

float sliceBuffers[MAX_SLICES][MAX_SLICE_LENGTH];
int sliceLengths[MAX_SLICES];  // Actual length of each slice
int slicePlayPositions[MAX_SLICES];  // Current playback position
```

**Memory Calculation:**
- 16 slices × 24,000 samples × 4 bytes = 1.536 MB
- Well within Daisy Seed 64MB SDRAM

**Variability:**
When K5 is set to 100ms but variability is enabled:
- Slice 1: 80ms (random -20%)
- Slice 2: 120ms (random +20%)
- Slice 3: 95ms (random -5%)
- Creates evolving, non-static patterns

---

### 5.3 Playback Sequencer

**Purpose:** Determines which slice plays, in what order, and how many times

**Parameters:**
- **STUTTER (K6):** Repetition probability/intensity (0-100%)
- **Playback Mode (Toggle 1):** Order of playback

**Stutter Implementation:**

```cpp
// Determine how many times to repeat current slice
int CalculateRepeatCount(float stutterAmount) {
    int baseRepeats = 1;
    
    // Random chance based on stutter amount
    if (random(0.0f, 1.0f) < stutterAmount) {
        int maxRepeats = (int)(stutterAmount * 8.0f);  // 0-8 range
        return baseRepeats + random(1, maxRepeats);
    }
    
    return baseRepeats;
}

// Playback loop
void PlaybackSequencer() {
    // Play current slice
    float output = sliceBuffers[currentSlice][playPosition++];
    
    // Check if reached end of slice
    if (playPosition >= sliceLengths[currentSlice]) {
        repeatCount++;
        playPosition = 0;  // Loop this slice
        
        // Check if done repeating
        if (repeatCount >= targetRepeats) {
            AdvanceToNextSlice();  // Based on Toggle 1 mode
            repeatCount = 0;
            targetRepeats = CalculateRepeatCount(stutterK6);
        }
    }
    
    return output;
}
```

**Playback Modes (Toggle 1):**

**UP - Forward Playback:**
```
Play order: 1 → 2 → 3 → 4 → 1 → 2 → 3 → 4...
Matches capture (if sequential capture)
```

**MIDDLE - Reverse Playback:**
```
Play order: 4 → 3 → 2 → 1 → 4 → 3 → 2 → 1...
Opposite of capture order
```

**DOWN - Random Playback:**
```
Play order: 2 → 4 → 1 → 3 → 4 → 2 → 1 → 4...
Chaotic, unpredictable
```

**Example with Stutter:**
```
4 slices, stutter = 70%
Playback: [1][1][1][2][3][3][3][3][4][4][1][1][2][2][2][3]...
           ↑ 3x    ↑  ↑ 4x      ↑ 2x ↑ 2x ↑ 3x    ↑
```

---

### 5.4 Per-Slice Processing (Toggle 2)

**Purpose:** Apply different effects to each slice

**UP - Clean:**
- No processing
- Pass through as captured
- Lowest CPU usage

**MIDDLE - Reverse:**
```cpp
// Play slice backwards
float ReverseSlice(int slice, int position) {
    int reversePos = sliceLengths[slice] - 1 - position;
    return sliceBuffers[slice][reversePos];
}
```
- Each slice plays backwards
- Creates reverse delay effect per chunk
- Smooth transitions between slices

**DOWN - Shimmer:**
```cpp
// Octave up + original
float ShimmerSlice(int slice, int position) {
    // Read at 2x speed for octave up
    int octavePos = (position * 2) % sliceLengths[slice];
    float octave = sliceBuffers[slice][octavePos];
    float original = sliceBuffers[slice][position];
    
    return (octave * 0.5f) + (original * 0.5f);
}
```
- Pitch shift up one octave
- Mix with original
- Ethereal shimmer effect

---

### 5.5 Shift Mode A - Modulation & Tone

**LFO Modulation (K1, K2, K3):**

```cpp
// LFO oscillator
Oscillator modLFO;
modLFO.SetFreq(K1);  // 0.05 - 10Hz
modLFO.SetAmp(K2);   // 0 - 100%

float lfoValue = modLFO.Process();  // -1.0 to 1.0

// Apply to target based on K3
if (K3 < 0.33f) {
    // Modulate slice length
    float modAmount = lfoValue * maxModDepth;
    sliceLength += (int)(modAmount * baseSliceLength);
    
} else if (K3 < 0.66f) {
    // Modulate filter cutoff
    float modAmount = lfoValue * maxFilterMod;
    filterCutoff = baseFilterCutoff + modAmount;
    
} else {
    // Modulate slice count
    float modAmount = lfoValue * maxCountMod;
    activeSliceCount = baseSliceCount + (int)modAmount;
}
```

**Filter (K4):**
- Low-pass filter on sliced output
- Smooths harsh digital edges
- 0% = bright/full bandwidth
- 100% = dark/muffled

**Drive (K5):**
- Soft clipping saturation
- Adds harmonic richness
- 0% = clean
- 100% = heavily saturated

**Diffusion (K6):**
- Smears slice boundaries
- Adds reverb-like texture
- Creates "cloud" quality
- 0% = discrete slices
- 100% = merged texture

---

### 5.6 Shift Mode B - Lo-Fi & Output

**Bit Crush (K2):**
```cpp
// Sample rate and bit depth reduction
float BitCrush(float input, float amount) {
    // Reduce sample rate
    int downsample = (int)(1.0f + amount * 15.0f);  // 1-16x reduction
    if (sampleCounter++ % downsample != 0) {
        return lastSample;  // Hold previous sample
    }
    
    // Reduce bit depth
    int bits = (int)((1.0f - amount) * 16.0f);  // 16 bits down to 1
    float step = 1.0f / powf(2.0f, bits);
    float crushed = floorf(input / step) * step;
    
    lastSample = crushed;
    return crushed;
}
```

**Wobble (K3):**
```cpp
// Tape wow/flutter - pitch instability
float Wobble(float input, float amount) {
    // Very slow LFO for wow
    float wow = sinf(wowPhase) * amount * 0.02f;
    wowPhase += 0.0001f;  // Very slow
    
    // Faster random flutter
    if (random() < 0.01f) {
        flutter = (random() - 0.5f) * amount * 0.01f;
    }
    
    // Modulate delay time slightly
    float pitchMod = 1.0f + wow + flutter;
    return ReadWithPitchMod(input, pitchMod);
}
```

**Noise (K4):**
```cpp
// Background hiss like tape
float AddNoise(float input, float amount) {
    float noise = (random() - 0.5f) * 2.0f;  // -1 to 1
    return input + (noise * amount * 0.1f);
}
```

**AGE Master Control (K6):**
```cpp
// Master lo-fi that affects multiple params
void ApplyAge(float ageAmount) {
    bitCrushAmount = ageAmount * 0.7f;  // Strong effect
    wobbleAmount = ageAmount * 0.5f;    // Medium effect
    noiseAmount = ageAmount * 0.3f;     // Subtle effect
    
    // Apply all three
    output = BitCrush(output, bitCrushAmount);
    output = Wobble(output, wobbleAmount);
    output = AddNoise(output, noiseAmount);
}
```

**Master Level (K1):**
- -12dB to +6dB range
- Compensate for feedback buildup
- Or push into saturation
- Essential for managing dynamics

**Spread (K5):**
- Stereo width control (if stereo build)
- 0% = mono
- 50% = natural stereo
- 100% = hyper-wide

---

### 5.7 Freeze Mode (FS2 in Normal Mode)

**Purpose:** Capture current slice pattern and loop it indefinitely

**Operation:**
```
1. User presses FS2
2. Current slice pattern is frozen
3. No new audio captured
4. Existing slices loop with:
   - Current stutter settings
   - Current playback order
   - Current effects
5. Can adjust knobs to modify frozen pattern
6. Press FS2 again to unfreeze
```

**LED Behavior:**
- LED2 solid = Frozen
- LED2 off = Active capture

**Use Cases:**
- Create drone from interesting moment
- Build layers by freezing and overdubbing
- Solo over frozen texture
- Create evolving pad by modulating frozen slices

---

## 6. MEMORY & CPU BUDGET

### Memory Requirements

**Slice Buffers:**
- Primary: 1.536 MB (16 slices × 24K samples)
- Working: ~200 KB (DSP state, intermediate buffers)
- **Total: ~1.75 MB** (within 64MB SDRAM)

**Optimization Options:**
- Reduce MAX_SLICES to 8 → 768 KB
- Reduce MAX_SLICE_LENGTH to 250ms → 768 KB
- Both → 384 KB (matches Phase 2)

### CPU Budget Estimate

| Processing Block | CPU % | Phase |
|-----------------|-------|-------|
| Slice Capture | 5% | 1 |
| Buffer Management | 5% | 1 |
| Playback Sequencer | 3% | 1 |
| Stutter Logic | 2% | 1 |
| Per-Slice Effects | 10% | 2 |
| LFO Modulation | 5% | 3 |
| Filter/Drive | 8% | 3 |
| Bit Crush | 5% | 4 |
| Wobble | 5% | 4 |
| Parameter Smoothing | 2% | Always |
| **TOTAL ESTIMATED** | **50%** | All |

**Headroom: 50%** for optimization and future features

---

## 7. DEVELOPMENT ROADMAP

### Phase 1: Core S&H Engine (3-4 weeks)

**Goals:** Basic slice capture and playback working

- [ ] Implement slice buffer array
- [ ] Create capture engine (sequential mode only)
- [ ] Implement playback sequencer (forward only)
- [ ] Basic slice count and length controls
- [ ] Dry/wet mix
- [ ] Bypass switching

**Deliverable:** Can capture and play back slices

---

### Phase 2: Stutter & Modes (2-3 weeks)

**Goals:** Add stutter control and playback modes

- [ ] Implement stutter probability system
- [ ] Add repeat count logic
- [ ] Implement reverse playback mode
- [ ] Implement random playback mode
- [ ] Add random/reverse capture modes
- [ ] Smooth transitions between modes

**Deliverable:** Full playback control with glitch effects

---

### Phase 3: Per-Slice Processing (2-3 weeks)

**Goals:** Different effects per slice

- [ ] Clean mode (pass through)
- [ ] Reverse mode (backwards playback)
- [ ] Shimmer mode (octave up)
- [ ] Crossfading between slices
- [ ] CPU optimization

**Deliverable:** Toggle 2 fully functional

---

### Phase 4: Shift Mode A - Modulation (2 weeks)

**Goals:** Add modulation and tone shaping

- [ ] LFO implementation (K1, K2)
- [ ] Modulation target routing (K3)
- [ ] Filter implementation (K4)
- [ ] Drive/saturation (K5)
- [ ] Diffusion processing (K6)
- [ ] Tap tempo for LFO (FS2)

**Deliverable:** Toggle 3 MIDDLE functional

---

### Phase 5: Shift Mode B - Lo-Fi (2 weeks)

**Goals:** Add lo-fi degradation and output control

- [ ] Bit crusher (K2)
- [ ] Wobble/flutter (K3)
- [ ] Noise generator (K4)
- [ ] Master level (K1)
- [ ] AGE master control (K6)
- [ ] Dry kill (FS2)

**Deliverable:** Toggle 3 DOWN functional

---

### Phase 6: Freeze Mode (1 week)

**Goals:** Implement freeze functionality

- [ ] Freeze capture logic
- [ ] Loop frozen pattern
- [ ] LED indication
- [ ] Parameter control while frozen
- [ ] Smooth freeze/unfreeze

**Deliverable:** FS2 freeze in Normal mode

---

### Phase 7: Polish & Optimization (2 weeks)

**Goals:** Prepare for release

- [ ] CPU optimization pass
- [ ] Memory optimization if needed
- [ ] Click/pop elimination
- [ ] Comprehensive testing
- [ ] Parameter range tuning
- [ ] LED behavior refinement
- [ ] User documentation

**Deliverable:** Release candidate

---

### Phase 8: Presets (Optional - 1 week)

**Goals:** Add preset storage

- [ ] QSPI flash storage
- [ ] Settings struct for all 18 parameters
- [ ] Save/load mechanism
- [ ] LED feedback
- [ ] 4-8 preset slots

**Deliverable:** Preset system

---

**Total Development Time: 14-17 weeks (~3.5-4 months)**

---

## 8. TECHNICAL SPECIFICATIONS

### Audio Specifications

- **Sample Rate:** 48 kHz
- **Bit Depth:** 32-bit float internal processing
- **Latency:** <5ms (buffer-dependent)
- **Frequency Response:** 20Hz - 20kHz (±1dB)
- **Dynamic Range:** >100dB
- **THD+N:** <0.1% @ 1kHz

### Hardware

- **Platform:** Cleveland Music Co. Hothouse (Daisy Seed)
- **DSP:** Daisy Seed (STM32H750, 480MHz)
- **Memory:** 64MB SDRAM
- **Storage:** 8MB QSPI Flash (for presets)
- **I/O:** Mono in/out (stereo possible in future)
- **Power:** 9V DC center-negative, ~150mA

### Controls

- **Knobs:** 6 (potentiometers)
- **Toggles:** 3 (3-position switches)
- **Footswitches:** 2 (momentary)
- **LEDs:** 2 (status indicators)

---

## 9. COMPARISON TO PHASE 2 (TREMOLO SLICER)

| Aspect | Phase 2 (Tremolo) | Phase 3 (S&H) |
|--------|-------------------|---------------|
| **Architecture** | Continuous + gate | Discrete capture |
| **CPU Usage** | ~15% | ~50% |
| **Memory** | 400KB | 1.75MB |
| **Feel** | Musical, rhythmic | Glitchy, experimental |
| **Complexity** | Simple | Complex |
| **Dev Time** | 1 month | 3.5 months |
| **Market** | General | Experimental |
| **Price** | $200-250 | $300-350 |

**Both are valid products for different users!**

---

## 10. USER SCENARIOS

### Scenario 1: Ambient Pad Creation

**Settings:**
- K4 SLICE COUNT: 8
- K5 SLICE LENGTH: 250ms (long, smooth)
- K6 STUTTER: 20% (occasional glitch)
- K3 FEEDBACK: 70% (long regeneration)
- Toggle 1: Forward/Forward
- Toggle 2: Shimmer (octave up)

**Result:** Smooth, evolving pad with occasional stutters and shimmer

---

### Scenario 2: Aggressive Glitch

**Settings:**
- K4 SLICE COUNT: 16 (maximum granularity)
- K5 SLICE LENGTH: 25ms (very short)
- K6 STUTTER: 95% (heavy repetition)
- K3 FEEDBACK: 90% (builds up)
- Toggle 1: Random/Random
- Toggle 2: Reverse

**Result:** Chaotic, stuttering, backwards chaos

---

### Scenario 3: Rhythmic Loop

**Settings:**
- K4 SLICE COUNT: 4 (quarter note feel)
- K5 SLICE LENGTH: 120ms (musical length)
- K6 STUTTER: 40% (some repeats)
- K3 FEEDBACK: 50% (moderate regen)
- Toggle 1: Forward/Forward
- Toggle 2: Clean
- FS2: FREEZE after capturing good pattern

**Result:** Locked rhythmic pattern, can solo over

---

### Scenario 4: Lo-Fi Texture

**Settings:**
- Toggle 3: DOWN (Shift Mode B)
- K6 AGE: 80% (heavy degradation)
- K2 BIT CRUSH: 60%
- K3 WOBBLE: 70%
- K4 NOISE: 50%

**Result:** Heavily degraded, tape-like sliced delays

---

## 11. QUICK REFERENCE

### Front Panel Diagram

```
┌─────────────────────────────────────────────────────────────┐
│          FLUX - Sample & Hold Slicer Delay                   │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   ①TIME      ②MIX       ③FEEDBACK                           │
│   [  ◁  ]    [  ◁  ]    [  ◁  ]                            │
│                                                              │
│   ④SLICE     ⑤SLICE     ⑥STUTTER                            │
│    COUNT      LENGTH                                         │
│   [  ◁  ]    [  ◁  ]    [  ◁  ]                            │
│                                                              │
│  SW1:CAP/PLAY  SW2:EFFECT   SW3:MODE                        │
│    ┃ ┃ ┃        ┃ ┃ ┃       ┃ ┃ ┃                          │
│   Fwd Rev Rnd  Cln Rev Shm  Nrm Mod Lofi                   │
│                                                              │
│    FS1:[BYPASS]         FS2:[FREEZE/TAP/DRY KILL]          │
│     (LED1)                    (LED2)                        │
└─────────────────────────────────────────────────────────────┘
```

### Mode Quick Reference

**NORMAL MODE (Toggle 3 UP):**
- K1-K6: Core slicing controls
- FS2: FREEZE

**SHIFT MODE A (Toggle 3 MIDDLE):**
- K1-K6: Modulation & tone
- FS2: TAP TEMPO

**SHIFT MODE B (Toggle 3 DOWN):**
- K1-K6: Lo-fi & output
- FS2: DRY KILL

---

## 12. FUTURE ENHANCEMENTS

### Phase 9+ (Optional)

**Stereo Operation:**
- Dual mono capture
- Stereo spread/panning
- L/R independent processing

**MIDI Integration:**
- PC for presets
- CC for all parameters
- Clock sync for tempo

**Advanced Patterns:**
- Euclidean rhythms
- Polyrhythmic divisions
- User-definable patterns

**Additional Effects:**
- Granular per-slice
- Resonator/physical modeling
- More reverb/delay types

**Expression Pedal:**
- Control any parameter
- Morph between settings
- Real-time performance

---

## 13. CREDITS & ATTRIBUTION

**Design:** FLUX Sample & Hold Slicer Delay v3.0  
**Platform:** Cleveland Music Co. Hothouse (Daisy Seed)  
**DSP Library:** Electro-Smith DaisySP  
**Inspiration:** 
- Chase Bliss Habit (memory-based manipulation)
- Chase Bliss Mood (dual-channel processing)
- Boss SL-2 (pattern-based slicing)
- MXR Layers (sustain/texture creation)

**Previous Work:**
- Phase 2 Tremolo Slicer (archived, still valuable)

**License:** MIT

---

**END OF SPECIFICATIONS v3.0**

*Ready to begin Phase 1 development of Sample & Hold architecture.*
