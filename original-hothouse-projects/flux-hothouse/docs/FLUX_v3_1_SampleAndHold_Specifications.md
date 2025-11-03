# FLUX v3.1: Sample & Hold Slicer Delay
## Updated Specifications - Post Phase 2 Completion

**Date:** 2025-10-29  
**Version:** 3.1 - Phase 2 Complete, Phase 5 Next  
**Previous:** v3.0 (Initial S&H architecture spec)

---

## DOCUMENT HISTORY

- **v1.0 (2025-10-15):** Initial tremolo slicer concept
- **v2.0 (2025-10-16):** Refined tremolo with shift modes
- **v3.0 (2025-10-26):** Complete pivot to Sample & Hold architecture
- **v3.1 (2025-10-29):** Updated after Phase 2 completion
  - K1 changed from pre-delay to master level control
  - Phase 5 (Lo-Fi Mode) prioritized as next development
  - Core slicing and stutter system validated and working
  - Clicking issues resolved with proper crossfading
  - K4/K5 control curves optimized

---

## 1. OVERVIEW

**Concept:** A Sample & Hold delay pedal that captures discrete audio slices, stores them in individual buffers, and plays them back with controllable repetition, randomization, and per-slice processing. Think "MPC meets Chase Bliss" - the deliberate stutter and glitch of a sampler with the experimental chaos of a boutique delay.

**Philosophy:** Transform guitar into a glitch machine by freezing discrete moments in time and manipulating them rhythmically. Instead of continuous delay with gating (tremolo style), we capture distinct audio snapshots that can be repeated, reordered, and processed independently.

**Current Status:** 
- ✅ **Phase 1 Complete** - Core slicing functional
- ✅ **Phase 2 Complete** - Stutter system and toggle modes working perfectly
- 🎯 **Next:** Phase 5 - Lo-Fi Mode (Toggle 3 DOWN)

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

---

## 3. CONTROL MAPPING

### 3.1 Normal Mode (Toggle 3 UP) - ✅ COMPLETE

#### Knobs - Core Slicing Parameters

| Knob | Parameter | Range | Function | Status | Notes |
|------|-----------|-------|----------|--------|-------|
| **K1** | **LEVEL** | -∞ to +6dB | Master output level | ✅ CHANGED | Was pre-delay, now level control |
| **K2** | MIX | 0% - 100% | Dry/wet blend | ✅ COMPLETE | Constant power crossfade |
| **K3** | FEEDBACK | 0% - 100% | Pattern regeneration | ✅ COMPLETE | Sliced audio feeds back |
| **K4** | SLICE COUNT | 1 - 16 | Number of slices per cycle | ✅ COMPLETE | Fixed rounding for full range |
| **K5** | SLICE LENGTH | 100ms - 500ms | Duration of each slice | ✅ COMPLETE | Log curve, min raised to 100ms |
| **K6** | STUTTER | 0% - 100% | Repetition probability | ✅ COMPLETE | Musical subdivisions working |

#### Toggle Switches

**Toggle 1: Capture/Playback Mode** - ✅ COMPLETE
- **UP:** Forward sequence / Forward playback
- **MIDDLE:** Backward sequence / Reverse playback  
- **DOWN:** Forward sequence / Random playback direction per slice

**Toggle 2: Per-Slice Effect** - ⏳ FUTURE
- **UP:** Clean (no processing)
- **MIDDLE:** Reverse (backwards playback per slice)
- **DOWN:** Shimmer (octave up per slice)

**Toggle 3: Mode Selector** - ✅ UP COMPLETE, DOWN NEXT
- **UP:** Normal Mode (core slicing controls) - ✅ COMPLETE
- **MIDDLE:** Shift Mode A (modulation & tone) - ⏳ FUTURE
- **DOWN:** Shift Mode B (lo-fi & output) - 🎯 NEXT PRIORITY

#### Footswitches

**FS1 (Left):** - ✅ COMPLETE
- **Tap:** Toggle bypass
- **Hold 2s:** Bootloader entry

**FS2 (Right) - Context Sensitive:** - ⏳ FUTURE
- **Normal Mode:** FREEZE (capture and loop current pattern)
- **LED2:** Shows freeze state (solid when frozen)

---

### 3.2 Shift Mode A (Toggle 3 MIDDLE) - Modulation & Tone - ⏳ FUTURE

#### Knobs - Modulation Parameters

| Knob | Parameter | Range | Function | Notes |
|------|-----------|-------|----------|-------|
| **K1** | MOD RATE | 0.05Hz - 10Hz | LFO speed | Slow drift to fast wobble |
| **K2** | MOD DEPTH | 0% - 100% | LFO intensity | How much modulation |
| **K3** | MOD TARGET | 0% - 100% | What gets modulated | See below |
| **K4** | TONE | 0% - 100% | Filter cutoff | Low-pass on slices |
| **K5** | DRIVE | 0% - 100% | Saturation amount | Distortion/overdrive |
| **K6** | DIFFUSION | 0% - 100% | Texture/smear | Adds "cloud" quality |

*(Implementation details same as v3.0 - deferred to Phase 4)*

---

### 3.3 Shift Mode B (Toggle 3 DOWN) - Lo-Fi & Output - 🎯 NEXT PRIORITY

#### Knobs - Lo-Fi Parameters

| Knob | Parameter | Range | Function | Implementation Notes |
|------|-----------|-------|----------|---------------------|
| **K1** | MASTER LEVEL | -∞ to +6dB | Overall output volume | Already in Normal mode, applies here too |
| **K2** | BIT CRUSH | 0% - 100% | Sample rate reduction | Digital degradation |
| **K3** | WOBBLE | 0% - 100% | Pitch instability | Tape wow/flutter |
| **K4** | NOISE | 0% - 100% | Background hiss | Analog tape character |
| **K5** | SPREAD | 0% - 100% | Stereo width | If stereo build, else reserved |
| **K6** | AGE | 0% - 100% | Master lo-fi amount | Controls K2,K3,K4 together |

#### K6 AGE Master Control

When turned up, simultaneously increases:
- Bit crush amount (K2)
- Wobble intensity (K3)
- Noise level (K4)
- Creates unified "degraded tape" effect

#### Toggle Behavior
- **Toggle 1 & 2:** Same as Normal Mode
- **Toggle 3:** DOWN position activates this mode

#### Footswitch Behavior
**FS2:** DRY KILL toggle
- Mutes dry signal, wet only
- **LED2:** Shows dry kill state (solid when dry killed)

---

## 4. DEVELOPMENT STATUS

### ✅ Phase 1: Foundation (COMPLETE - 2025-10-26)

**Completed:**
- ✅ Hardware initialization
- ✅ Bypass switching
- ✅ LED indicators
- ✅ Control processing structure
- ✅ Bootloader entry
- ✅ Slice buffer array structure
- ✅ K2: DRY/WET MIX (0-100%)
- ✅ K3: FEEDBACK (0-100%)
- ✅ K4: SLICE COUNT (1-16) with proper rounding
- ✅ K5: SLICE LENGTH (100-500ms) with logarithmic curve
- ✅ Zero-crossing detection for click-free capture

**Key Artifacts:**
- `flux_main.cpp` - Initial working version

---

### ✅ Phase 2: Stutter & Modes (COMPLETE - 2025-10-29)

**Completed:**
- ✅ K6: STUTTER control (0-100%)
- ✅ Shuffle probability (0-100% based on K6)
- ✅ Musical subdivision repeats (1x, 2x, 4x, 8x)
- ✅ Random number generator seeded with system time
- ✅ Toggle 1 modes working correctly
- ✅ Variable crossfade system (15% with 5ms minimum)
- ✅ Read/write conflict protection
- ✅ Fixed K4 dead zone (proper rounding)
- ✅ Fixed K5 curve (true logarithmic)
- ✅ Raised minimum slice length to 100ms for musical content

**Key Lessons:**
- Playback position MUST reset to 0 on slice advance
- Read/write conflicts cause clicking when currentPlaybackSlice == currentCaptureSlice
- Symmetrical fade in/out required at boundaries
- Variable crossfade proportional to slice length works better than fixed
- Minimum 100ms slice ensures musical content after crossfades

**Key Artifacts:**
- `flux_main_fixed_step5.cpp` - Current working version (2025-10-29)

**Testing Results:**
- ✅ Sounds "incredible" 
- ✅ K4 responsive across full range
- ✅ K5 proper logarithmic control feel
- ✅ K6 stutter working as designed (clean at 0%, chaotic at 100%)
- ✅ Clicking eliminated
- ✅ "Very cool ambient delay"

---

### 🎯 Phase 5: Lo-Fi Mode (NEXT PRIORITY - 1-2 weeks)

**Goals:** Implement Toggle 3 DOWN shift mode

**Tasks:**
- [ ] Detect Toggle 3 position and switch control behavior
- [ ] Bit crush implementation (K2)
  - Sample rate reduction algorithm
  - 0% = clean, 100% = heavy reduction
- [ ] Wobble/flutter implementation (K3)
  - Pitch instability using LFO
  - Simulate tape wow/flutter
- [ ] Background noise (K4)
  - White/pink noise generator
  - Mix with output signal
- [ ] AGE master control (K6)
  - Unified control of K2, K3, K4
  - Creates cohesive "degraded" effect
- [ ] Dry kill (FS2 in this mode)
  - Toggle dry signal on/off
  - LED2 indication
- [ ] K1 MASTER LEVEL implementation
  - Apply in both Normal and Lo-Fi modes
  - -∞ to +6dB range

**Implementation Strategy:**
1. Add mode detection based on Toggle 3
2. Implement bit crusher first (simplest)
3. Add wobble with slow LFO
4. Add noise generator
5. Wire up AGE master control
6. Implement dry kill toggle
7. Test all Lo-Fi parameters together

**Deliverable:** Toggle 3 DOWN functional, "degraded tape" aesthetic working

---

### ⏳ Phase 3: Per-Slice Effects (DEFERRED)

**Goals:** Implement Toggle 2 functionality
- [ ] Clean mode (pass-through)
- [ ] Reverse mode (backwards playback per slice)
- [ ] Shimmer mode (octave up per slice)

**Deliverable:** Toggle 2 functional

---

### ⏳ Phase 4: Modulation Mode (DEFERRED)

**Goals:** Implement Toggle 3 MIDDLE shift mode
- [ ] LFO system (K1, K2, K3)
- [ ] Tone control (K4)
- [ ] Drive/saturation (K5)
- [ ] Diffusion (K6)
- [ ] Tap tempo (FS2)

**Deliverable:** Toggle 3 MIDDLE functional

---

### ⏳ Phase 6: Freeze Mode (DEFERRED)

**Goals:** Implement freeze functionality
- [ ] Freeze capture logic
- [ ] Loop frozen pattern
- [ ] LED indication
- [ ] Parameter control while frozen

**Deliverable:** FS2 freeze in Normal mode

---

### ⏳ Phase 7: Polish & Optimization (FUTURE)

**Goals:** Prepare for release
- [ ] CPU optimization pass
- [ ] Memory optimization
- [ ] Comprehensive testing
- [ ] Parameter range tuning
- [ ] User documentation

**Deliverable:** Release candidate

---

## 5. TECHNICAL SPECIFICATIONS

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

## 6. GITHUB REPOSITORY PLAN

### Repository Structure

```
daisy-pedals/
├── README.md
├── LICENSE
├── .gitignore
├── platformio.ini
├── lib/
│   └── (Cleveland Music Co. Hothouse library as submodule)
├── flux/
│   ├── README.md
│   ├── docs/
│   │   ├── FLUX_v3_1_SampleAndHold_Specifications.md
│   │   └── changelog.md
│   ├── src/
│   │   └── flux_main.cpp
│   ├── snapshots/
│   │   ├── flux_phase1_complete.cpp (2025-10-26)
│   │   └── flux_phase2_complete.cpp (2025-10-29)
│   └── platformio.ini
├── mars/
│   └── (neural amp modeling project)
├── venus/
│   └── (spectral reverb project)
└── earth/
    └── (reverbscape project)
```

### Submodules

- Cleveland Music Co. Hothouse library
- DaisySP (if not included in Hothouse)
- libDaisy (if not included in Hothouse)

### Tagging Strategy

- **Phase Completion Tags:** `flux-phase1-complete`, `flux-phase2-complete`, etc.
- **Version Tags:** `flux-v3.0`, `flux-v3.1`, etc.
- **Feature Tags:** `flux-lofi-mode`, `flux-freeze-mode`, etc.

### Next Steps for Git Setup

1. Initialize repository with proper .gitignore
2. Add Cleveland Music Co. library as submodule
3. Create initial commit with Phase 2 complete code
4. Tag current state as `flux-phase2-complete` and `flux-v3.1`
5. Set up branches for future work (`dev`, `lofi-mode`, etc.)

---

## 7. USER SCENARIOS

### Scenario 1: Ambient Pad Creation (VALIDATED ✅)

**Settings:**
- K4 SLICE COUNT: 8
- K5 SLICE LENGTH: 250ms (long, smooth)
- K6 STUTTER: 20% (occasional glitch)
- K3 FEEDBACK: 70% (long regeneration)
- Toggle 1: Forward/Forward

**Result:** "Very cool ambient delay" - smooth, evolving pad with occasional stutters

---

### Scenario 2: Aggressive Glitch

**Settings:**
- K4 SLICE COUNT: 16 (maximum granularity)
- K5 SLICE LENGTH: 100ms (shortest)
- K6 STUTTER: 95% (heavy repetition)
- K3 FEEDBACK: 90% (builds up)
- Toggle 1: DOWN (random direction per slice)

**Result:** Chaotic, stuttering, unpredictable sonic textures

---

### Scenario 3: Lo-Fi Texture (TARGET FOR PHASE 5)

**Settings:**
- Toggle 3: DOWN (Shift Mode B)
- K6 AGE: 80% (heavy degradation)
- K2 BIT CRUSH: Auto from AGE
- K3 WOBBLE: Auto from AGE
- K4 NOISE: Auto from AGE

**Result:** Heavily degraded, tape-like sliced delays

---

## 8. QUICK REFERENCE

### Current Working Features (Phase 2 Complete)

```
┌─────────────────────────────────────────────────────────────┐
│          FLUX - Sample & Hold Slicer Delay v3.1              │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   ①LEVEL     ②MIX       ③FEEDBACK        ✅ WORKING         │
│   [  ◁  ]    [  ◁  ]    [  ◁  ]                            │
│                                                              │
│   ④SLICE     ⑤SLICE     ⑥STUTTER         ✅ WORKING         │
│    COUNT      LENGTH                                         │
│   [  ◁  ]    [  ◁  ]    [  ◁  ]                            │
│                                                              │
│  SW1:CAP/PLAY  SW2:EFFECT   SW3:MODE     ✅ SW1 WORKING     │
│    ┃ ┃ ┃        ┃ ┃ ┃       ┃ ┃ ┃       ⏳ SW2 FUTURE      │
│   Fwd Bwd Rnd  Cln Rev Shm  Nrm Mod Lofi ⏳ SW3 MID/DOWN    │
│                                                              │
│    FS1:[BYPASS]         FS2:[RESERVED]   ✅ FS1 WORKING     │
│     (LED1)                    (LED2)     ⏳ FS2 FUTURE      │
└─────────────────────────────────────────────────────────────┘
```

---

## 9. NEXT SESSION PLAN

### Immediate Priority: Lo-Fi Mode Implementation

**Session Goals:**
1. Review current code architecture
2. Implement Toggle 3 position detection
3. Design bit crusher algorithm
4. Implement wobble/flutter using LFO
5. Add noise generator
6. Wire up AGE master control
7. Test Lo-Fi mode thoroughly

**Estimated Time:** 1-2 weeks

**Success Criteria:**
- Toggle 3 DOWN activates Lo-Fi mode
- All Lo-Fi knobs functional
- "Degraded tape" aesthetic achieved
- No audio artifacts or glitches

---

## 10. CREDITS & ATTRIBUTION

**Design:** FLUX Sample & Hold Slicer Delay v3.1  
**Platform:** Cleveland Music Co. Hothouse (Daisy Seed)  
**DSP Library:** Electro-Smith DaisySP  
**Developer:** Chris Brandt (chris@futr.tv)
**Development Tool:** Claude.ai + daisy-agent

**Inspiration:** 
- Chase Bliss Habit (memory-based manipulation)
- Boss SL-2 (pattern-based slicing)
- MXR Layers (sustain/texture creation)

**License:** MIT

---

**END OF SPECIFICATIONS v3.1**

*Phase 2 Complete - Ready for Phase 5 (Lo-Fi Mode)*
