# Funbox → Hothouse DSP Migration — Full Reference

> Complete conversion guide and DSP pattern catalog. The essentials are summarized in `.claude/rules/dsp-conversion.md` (auto-loaded on .cpp/.h); this is the on-demand detail.


This document provides technical patterns for converting Daisy Seed DSP projects from Funbox to Hothouse architecture, compiled from successful Venus, Mars, Earth, and BuzzBox conversions.

## Performance Optimizations

### From Mars Developer - Pushing Daisy Seed to the Limit

The Mars developer shared these critical optimizations for maximum performance:

#### 1. Compiler Optimization
```makefile
# Use -Ofast for maximum optimization
OPT = -Ofast  # Instead of -Os or -O2
```

#### 2. CPU Boost
```cpp
// Pass true to Init for 480MHz (vs 400MHz default)
hw.Init(true);
```

#### 3. Audio Block Size
```cpp
// Use maximum block size for efficiency
hw.SetAudioBlockSize(256);  # Instead of default 48
```

#### 4. Performance Trade-offs

The Mars developer noted that Mars (neural + IR) pushes the Daisy to its limits:
- Neural model + IR = at maximum capacity
- Any additional effects would require optimization
- Some effects may need to run at reduced sample rates
- Profile code to identify bottlenecks

## Hardware Conversion Patterns

### Phase 1: Project Setup

**Create project directory structure:**
```
project_hothouse/
â”œâ”€â”€ main.cpp              (your converted code)
â”œâ”€â”€ hothouse.cpp          (copied from working Hothouse repo)
â”œâ”€â”€ hothouse.h            (copied from working Hothouse repo)
â”œâ”€â”€ Makefile              (updated for Hothouse)
â””â”€â”€ [other project files] (DSP headers, data files, etc.)
```

**Essential file requirements:**
- **`hothouse.cpp`**: Must be copied from working Hothouse repository
- **`hothouse.h`**: Must match the .cpp file version
- **Original DSP files**: All project-specific headers and data files

### Phase 2: Update Includes and Namespace

**Replace Funbox includes:**
```cpp
// OLD (Funbox)
#include "funbox.h"
using namespace funbox;

// NEW (Hothouse) 
#include "hothouse.h"
using namespace clevelandmusicco;
```

**Update hardware object:**
```cpp
// OLD (Funbox)
DaisyPetal hw;

// NEW (Hothouse) 
Hothouse hw;
```

### Phase 3: Convert Hardware Interface

#### Control Processing (Most Critical)

**Replace Funbox control processing:**
```cpp
// OLD (Funbox)
hw.ProcessAnalogControls();
hw.ProcessDigitalControls();

// NEW (Hothouse) 
hw.ProcessAllControls();  // Single method handles everything
```

#### Knob Reading

**Replace knob access:**
```cpp
// OLD (Funbox)
float knob1 = hw.knob[KNOB_1].Process();

// NEW (Hothouse)
float knob1 = hw.GetKnobValue(Hothouse::KNOB_1);
```

**All 6 knobs:**
```cpp
knobValues[0] = hw.GetKnobValue(Hothouse::KNOB_1);
knobValues[1] = hw.GetKnobValue(Hothouse::KNOB_2);
knobValues[2] = hw.GetKnobValue(Hothouse::KNOB_3);
knobValues[3] = hw.GetKnobValue(Hothouse::KNOB_4);
knobValues[4] = hw.GetKnobValue(Hothouse::KNOB_5);
knobValues[5] = hw.GetKnobValue(Hothouse::KNOB_6);
```

#### Toggle Switches

**Replace toggle switch reading:**
```cpp
// OLD (Funbox)
int toggle1 = hw.toggle[0].Pressed();

// NEW (Hothouse)
int toggle1 = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
```

**All 3 toggle switches:**
```cpp
toggleValues[0] = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
toggleValues[1] = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2);  
toggleValues[2] = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3);
```

#### Footswitches

**Replace footswitch handling:**
```cpp
// OLD (Funbox)
if(hw.switches[FOOTSWITCH_1].RisingEdge()) {
    bypass = !bypass;
}

// NEW (Hothouse)
if(hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
    bypass = !bypass;
}
```

#### LEDs

**Replace LED initialization and control:**
```cpp
// LED initialization in main()
led1.Init(hw.seed.GetPin(Hothouse::LED_1), false);
led2.Init(hw.seed.GetPin(Hothouse::LED_2), false);

// LED control in ProcessControls()
led1.Set(bypass ? 0.0f : 1.0f);
led2.Set(some_condition ? 1.0f : 0.0f);
led1.Update();
led2.Update();
```

### Phase 4: Analyze and Preserve Original DSP Implementation (Critical)

**Before making ANY changes to DSP processing, carefully analyze the original:**

1. **Document the signal path:**
```cpp
// Example from BuzzBox - document the exact order
// 1. Bass Boost (pre-gain)
// 2. Input Gain
// 3. Fuzz Drive
// 4. Soft Clipping
// 5. Tone Filter
// 6. Gate (if enabled)
```

2. **Identify parameter curves and ranges:**
```cpp
// Document exact formulas, even if they seem "wrong"
float bass_freq = 80.0f + (knobValues[2] * 120.0f);  // 80-200 Hz
float gain = 1.0f + (knobValues[0] * knobValues[0] * 29.0f);  // Squared curve
```

3. **Note any "backwards" or counterintuitive code:**
```cpp
// Earth.cpp example - volume REDUCTION is intentional!
if (odOn) {
    // This "backwards" formula creates a bloom/fade effect
    float od_reduction = 1.0f - (current_ODswell * current_ODswell * 2.8f - 0.1296f);
    effectOut = overdrive.Process(reverbOut * 0.25f) * od_reduction;
    // Volume decreases as overdrive increases - this is the desired effect!
}
```

4. **Understand design intent:**
```cpp
// Example: Tone control placement
// Fuzz pedals: Simple LP after clipping (vintage topology)
// Distortion pedals: Tilt-tone after clipping (modern voicing)
```

5. **Analyze gate architecture:**
```cpp
// Simple gate inside processing often works best
// Detects on gained signal (after bass boost and gain)
// Applied immediately to clipped signal
// Threshold calibrated for gained signal level (not raw input)

// Don't overcomplicate with separate detection/application
// unless there's a specific reason
```

### Phase 5: Update Makefile

**Essential Makefile structure:**
```makefile
# Project Name
TARGET = project_hothouse

USE_DAISYSP_LGPL = 1

# Memory configuration - QSPI for complex projects
APP_TYPE = BOOT_QSPI

# Compiler options - use -Ofast for maximum performance
OPT = -Ofast

# Sources - MUST include hothouse.cpp
CPP_SOURCES = main.cpp hothouse.cpp [other_sources.cpp]

# Library Locations
LIBDAISY_DIR = ../../libDaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic Makefile
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# Include directories
C_INCLUDES += -I.
```

**Key Makefile points:**
- **Always include `hothouse.cpp`** in CPP_SOURCES
- **Use `APP_TYPE = BOOT_QSPI`** for complex projects (requires bootloader)
- **Comment out for SRAM mode** during testing (no bootloader needed)
- **Use `-Ofast`** for maximum performance
- **Keep RTNeural/external dependencies minimal** until basic version works

### Phase 6: Create Debug Version First

**Always start with simplified debug version:**
```cpp
void ProcessControls() {
    hw.ProcessAllControls();
    
    // Test basic knob reading
    knobValues[0] = hw.GetKnobValue(Hothouse::KNOB_1);
    
    // Test basic footswitch
    if(hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }
    
    // Test LEDs
    led1.Set(bypass ? 0.0f : 1.0f);
    led1.Update();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {
    ProcessControls();
    
    for (size_t i = 0; i < size; i++) {
        float input = in[0][i];
        float output = bypass ? input : input * knobValues[0]; // Simple gain test
        
        out[0][i] = output;
        out[1][i] = output;
    }
}
```

### Phase 7: Add Complexity Incrementally

1. **Get basic version working** (bypass + simple gain)
2. **Add DSP processing** (effects, filters, etc.)
3. **Add advanced features** (neural networks, complex algorithms)
4. **Integrate multiple components** (multirate processing, multiple effects)

## Advanced DSP Techniques

### Envelope Follower Frequency Response

#### The Problem

Envelope followers (for autowah, dynamics, etc.) are naturally more sensitive to low frequencies than high frequencies. Bass notes dominate the envelope, while high notes on the high E string may not trigger properly.

#### Solution: Pre-Emphasis High-Pass Filter

**Professional technique** used in pedals like MXR Envelope Filter:

```cpp
// Add high-pass filter for detection only
ATone autowah_detector_hpf;

// Initialize (in main)
autowah_detector_hpf.Init(samplerate);
float detector_hpf_freq = 400.0f;  // 300-500Hz typical range
autowah_detector_hpf.SetFreq(detector_hpf_freq);

// In audio callback - filter BEFORE envelope detection
float detection_signal = autowah_detector_hpf.Process(signal);
float envelope = envelopeFollower.Process(detection_signal);  // Use filtered signal

// Original signal unchanged - only detection is filtered
autowah_svf.Process(signal);  // Process with full frequency content
```

**Key points:**
- HPF removes bass frequencies from detection signal only
- Original audio signal remains unaffected (full frequency content)
- Typical HPF frequencies: 200-500Hz
  - 200Hz: Gentle - still responds to bass
  - 300Hz: Balanced (good starting point)
  - 400Hz: Aggressive - emphasizes highs
  - 500Hz: Very aggressive - bass barely triggers
- May need to lower threshold to compensate for reduced signal level
- Creates even triggering across the entire fretboard

### Static Resonant Filter Mode

**Creative Use: Always-Open Gate**

```cpp
// When threshold is at 0, gate is always open
bool gate;
if (autowah_threshold > 0.01f) {
    float gate_level = 0.01f + (autowah_threshold * 0.11f);
    gate = (envelope > gate_level);
} else {
    gate = true;  // Gate always open - static filter
}
float adsr_out = autowah_adsr.Process(gate);
```

**Behavior:**
- Threshold at 0: ADSR stays at sustain level (e.g., 75%)
- Creates static resonant filter at fixed frequency
- Like a "cocked wah" pedal that doesn't move
- Range control sweeps the static filter frequency
- Different character from dynamic envelope following
- Useful creative option, not a bug

### Double-Filtering with Stateful Objects

**Problem:**
Calling the same stateful DSP object (like `Tone`) multiple times in one audio cycle can cause instability:

```cpp
// BAD - causes squealing/feedback/instability
if (fuzz_enabled) {
    tone.SetFreq(tone_freq);
    signal = tone.Process(signal);  // First use
}

// Later in same audio cycle...
if (fuzz_enabled && (autowah_enabled || octave_enabled)) {
    tone.SetFreq(tone_freq);
    signal = tone.Process(signal);  // Second use - UNSTABLE!
}
```

**Why it fails:**
- Tone filter has internal state (buffers, coefficients)
- Calling `SetFreq()` twice accumulates state
- Processing same signal twice creates resonant feedback
- Phase accumulation leads to oscillation

**Solution:**
- Use separate filter objects if you need double filtering
- OR accept the tonal shift as "part of the charm"
- OR apply filter only once per audio cycle

### ATone Filter Syntax

The `ATone::SetFreq()` method requires a variable reference, not a literal:

```cpp
// WRONG - won't compile
autowah_detector_hpf.SetFreq(400.0f);  // Error: cannot bind rvalue

// CORRECT
float detector_hpf_freq = 400.0f;
autowah_detector_hpf.SetFreq(detector_hpf_freq);  // Works
```

## Signal Path Architecture

### Vintage Fuzz Topology (Tone Bender, Fuzz Face)

**Critical ordering for authentic fuzz character:**

```cpp
// CORRECT order for vintage fuzz
float signal = input;

// 1. Bass boost BEFORE gain (shapes clipping character)
signal += bassBoost.Process(input);

// 2. Apply gain
signal *= drive_amount;

// 3. Clip the gained+boosted signal
signal = soft_clip(signal);

// 4. Tone control AFTER clipping (vintage topology)
signal = tone_filter.Process(signal);
```

**Why this order matters:**
- Bass boost before gain = fat, warm clipping
- Bass boost after clipping = muddy, indistinct
- Tone after clipping = vintage response
- Tone before clipping = modern distortion

### Frequency Ranges by Effect Type

**Fuzz pedals:**
```cpp
float tone_freq = 100.0f + (knob * 1400.0f);  // 100-1500 Hz range
```

**Distortion pedals:**
```cpp
float tone_freq = 400.0f + (knob * 2600.0f);  // 400-3000 Hz range
```

**Autowah:**
```cpp
float min_freq = 200.0f;   // Low end
float max_freq = 2500.0f;  // High end
float sweep_range = max_freq - min_freq;
```

### Tone Control Types

**Simple Low-Pass (for fuzz):**
```cpp
Tone tone_filter;
tone_filter.Init(samplerate);
tone_filter.SetFreq(tone_freq);  // Single frequency
```

**Tilt-Tone (for distortion):**
```cpp
// Simultaneous HP and LP for full tonal shaping
// Center frequency around 1kHz
// Boost highs while cutting lows, or vice versa
```

## Multirate Processing

### Critical Buffer Alignment

When mixing effects at different sample rates:

```cpp
// Example: 48kHz reverb + 24kHz octave generator
// Process reverb at full rate
reverbOut = reverb.Process(input);

// Octave runs at half rate - only process every other sample
if (phase == 0) {
    octaveOut = octave.Process(input);
}

// Mix outputs - ensure octave value persists between updates
float mixed = reverbOut + octaveOut;
```

**Key points:**
- Declare buffer variables outside audio callback
- Initialize to 0.0f
- Update only when processing
- Value persists between updates
- No clicks/pops if done correctly

### Q DSP Library for Downsampling

For more complex multirate needs:

```cpp
#include <q/support/decibel.hpp>
#include <q/fx/decimator.hpp>

// 2x downsampling
q::decimator<2> down_sampler;
```

## Gate Implementation

### Simple Internal Gate (Recommended)

```cpp
void ProcessAudio() {
    // 1. Apply bass boost and gain
    float signal = (input + bass_boost) * gain;
    
    // 2. Clip
    signal = soft_clip(signal);
    
    // 3. Gate the clipped signal
    if (gate_enabled) {
        float envelope = envelope_follower.Process(signal);
        if (envelope < gate_threshold) {
            signal = 0.0f;  // Kill signal below threshold
        }
    }
    
    // 4. Tone control
    output = tone_filter.Process(signal);
}
```

**Key points:**
- Detect on **gained signal** (after bass boost and gain)
- Threshold must match signal level at detection point
- Apply immediately to processed output
- Don't overcomplicate with separate detection/application unless needed

## UI/UX Design Patterns

### Context-Dependent Control Layout

When implementing multi-mode controls (like BuzzBox's Switch 3 changing Knob 4-6 functions), maintain consistency for similar functions:

**Good: Consistent layout**
```cpp
// Fuzz Mode
// Knob 4: Drive
// Knob 5: Tone
// Knob 6: Gate âœ…

// Autowah Mode
// Knob 4: Speed
// Knob 5: Range
// Knob 6: Threshold/Gate âœ…  (same position as Fuzz gate)

// Octave Mode
// Knob 4: Octave Up
// Knob 5: Octave Down
// Knob 6: Mix
```

**Benefits:**
- Better muscle memory
- Logical grouping (gate/threshold always on Knob 6)
- Less confusing when switching modes

### Sensible Defaults

Users should get usable sound immediately:

```cpp
// Initialize with musical defaults
float input_gain = 2.0f;      // Modest boost
float tone_freq = 800.0f;      // Mid-range
float gate_threshold = 0.05f;  // Light gating
```

### Document Complex Gain Structures

Multi-effect pedals often have complex, interactive gain structures. Document this clearly for users:

**In README/documentation:**
```markdown
## Gain Structure & Interaction

This pedal has a complex gain structure with multiple interactive stages:

- **Input Gain** (Knob 1): Affects everything downstream
- **Fuzz Drive**: Adds significant gain before clipping
- **Octave Processing**: Has individual level controls (up/down/mix)
- **Autowah Bandpass**: Naturally reduces volume (2x makeup gain applied)
- **FS2 Compensation**: 2x gain when effects active but fuzz bypassed
- **Dry/Wet Mix** (Knob 2): Blends processed with clean
- **Output Level** (Knob 3): Final stage control

**All stages interact:** Changing input gain affects fuzz character, octave behavior, 
and autowah sensitivity. Extreme settings can create unusual tones - this is 
intentional design space for exploration.
```

## Common Issues and Solutions

### Build Errors

**"No rule to make target hothouse.o"**
- **Solution**: Copy `hothouse.cpp` to project directory
- **Check**: `CPP_SOURCES` includes `hothouse.cpp`

**"Ambiguous function call" for DelayLine::SetDelay()**
- **Solution**: Use `float` type for delay parameters, not `int`
```cpp
struct SimpleDelay {
    float delayTarget;  // Use float, not int
    del->SetDelay(delayTarget);  // Now unambiguous
};
```

**RTNeural/External library errors**
- **Solution**: Remove complex dependencies for basic version
- **Strategy**: Get hardware working first, add libraries later

### DFU Programming Errors

**"Last page not writeable" or memory errors**
- **Solution**: Install Daisy bootloader first: `make program-boot`
- **Then**: Use `APP_TYPE = BOOT_QSPI` in Makefile
- **Verify**: DFU mode (hold BOOT, press/release RESET, release BOOT)

### Hardware Interface Issues

**Toggle switches not responding or stuck in one position**
- **Check**: Using exact API: `hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)`
- **Debug**: Create simple LED test to verify toggle states
- **Verify**: `hothouse.cpp` is properly compiled and linked
- **Test**: Switch positions may be inverted (case 0=up, case 2=down)

**Controls don't work**
- **Check**: Using `hw.ProcessAllControls()` not separate methods
- **Check**: `hothouse.cpp` file is present and compiling
- **Debug**: Create simple LED blink test to verify hardware

**LEDs don't work**
- **Check**: Using `hw.seed.GetPin(Hothouse::LED_1)` for initialization
- **Check**: Calling `led1.Update()` after `led1.Set()`
- **Check**: LED updates are in audio callback, not main loop

### Audio Processing Issues

**Effect Sounds Wrong / Weak Bass / Poor Tone Control**
- **Check**: Signal path order - is bass boost BEFORE or AFTER clipping?
- **Check**: Tone control type - simple LP for fuzz, tilt-tone for distortion?
- **Check**: Frequency ranges - fuzz needs 100-1500Hz, not 400-3000Hz
- **Solution**: Study vintage circuit topology (Tone Bender, Fuzz Face, etc.)

**Envelope follower not triggering on high notes**
- **Check**: Are low frequencies dominating the envelope?
- **Solution**: Add high-pass filter (300-500Hz) BEFORE envelope detection
- **Adjust**: Lower threshold to compensate for reduced signal level

**Clicks/pops when enabling effects**
- **Check**: Are buffers cleared when effects are off?
- **Solution**: Initialize all buffers to 0.0f when effect is bypassed
- **Check**: Parameter smoothing for gradual transitions

**Unexpected resonance or squealing**
- **Check**: Are you processing same signal through same filter object twice?
- **Solution**: Use separate filter instances or apply filter only once per cycle

## Build and Test Process

### Standard Build Commands

```bash
# 1. Prepare files
cp hothouse.cpp project_directory/
cp hothouse.h project_directory/

# 2. Clean build
make clean
make

# 3. Install bootloader (first time only for QSPI)
make program-boot

# 4. Program application  
make program-dfu
```

### Testing Checklist

1. **LEDs respond** - Basic hardware confirmed
2. **Footswitches toggle bypass** - Digital inputs working
3. **Knobs affect audio** - Analog inputs working
4. **Toggle switches change behavior** - All inputs functional
5. **Audio processing works** - DSP pipeline operational
6. **No unwanted distortion** - Buffers properly cleared
7. **Volume levels balanced** - Wet/dry mix appropriate
8. **Multirate processing clean** - No clicks or artifacts
9. **Signal path correct** - Bass boost before clipping for fuzz
10. **Tone control appropriate** - Simple LP for fuzz, tilt-tone for distortion
11. **Gate working** - Kills whine without cutting playing
12. **Envelope triggers properly** - High notes trigger autowah

## Debugging Strategies

### Systematic Component Testing

Test each component in isolation before integration:
```cpp
// Test 1: Basic audio passthrough
// Test 2: Add bass boost
// Test 3: Add gain
// Test 4: Add clipping
// Test 5: Add tone control
// Test 6: Test complete signal path
```

### Signal Path Verification

```cpp
void DebugSignalPath() {
    // Test order by adding one stage at a time
    // Stage 1: Input only
    output = input;
    
    // Stage 2: Add bass boost
    output = input + bass_boost;
    
    // Stage 3: Add gain
    output = (input + bass_boost) * gain;
    
    // Stage 4: Add clipping
    output = fuzz_clip((input + bass_boost) * gain);
    
    // Stage 5: Add tone
    output = tone_filter(fuzz_clip((input + bass_boost) * gain));
    
    // Listen at each stage to verify signal path
}
```

### Buffer State Verification

```cpp
void DebugBufferState() {
    // Check for uninitialized data
    for(int i = 0; i < 6; i++) {
        if(isnan(buff[i]) || isinf(buff[i])) {
            // Buffer has invalid data - initialize it!
            buff[i] = 0.0f;
        }
    }
}
```

### Effect Isolation Testing

```cpp
// Temporarily disable effects to isolate issues
bool test_reverb_only = true;
bool test_octave_only = false;
bool test_overdrive_only = false;

if(test_reverb_only) {
    // Process only reverb, bypass other effects
}
```

## Proven Successful Pattern

Based on Venus, Mars, Earth, and BuzzBox conversions:

1. **Copy working Hothouse library files** - Never recreate from scratch
2. **Analyze original implementation thoroughly** - Document exact types, curves, formulas, signal paths
3. **Understand design intent** - Test before "fixing" counterintuitive code
4. **Start with simple debug version** - Hardware interface first
5. **Use `hw.ProcessAllControls()` method** - Single control processing call
6. **Use exact API methods** - `hw.GetKnobValue()`, `hw.GetToggleswitchPosition()`
7. **Preserve original DSP structure exactly** - Including "backwards" formulas
8. **Include `hothouse.cpp` in build** - Essential for linking
9. **Test incrementally** - Verify each component before proceeding
10. **Handle multirate processing carefully** - Buffer alignment is critical
11. **Clear buffers when effects are off** - Prevent noise/artifacts
12. **Keep heavy operations out of audio callback** - Use main loop for bootloader
13. **Process controls in correct order** - Controls â†’ Buttons â†’ LEDs â†’ Audio
14. **Add complexity only after basics work** - Foundation first, features second
15. **Respect signal path topology** - Bass â†’ Gain â†’ Clip â†’ Tone for vintage fuzz
16. **Use appropriate frequency ranges** - Match range to effect type
17. **Choose correct tone control type** - Simple LP vs tilt-tone
18. **Keep gate architecture simple** - Don't overcomplicate
19. **Set sensible defaults** - Users get usable sound immediately
20. **Maintain UI consistency** - Similar functions on same knobs across modes
21. **Use pre-emphasis filtering** - For even envelope follower response
22. **Document complex interactions** - Gain structures, signal flow, creative features

This approach has a 100% success rate when followed completely. The key insights are to leverage the proven Hothouse library, preserve exact original implementation details (even when counterintuitive), respect signal path topology for vintage circuits, and systematically test each component before integration.

## Version History

- **v8.0** (October 2025) - Removed STAGE declarations and session management content (now in claude-rules-v2.0), focused on pure technical DSP patterns
- **v7.0** (October 2025) - Added BuzzBox learnings: envelope follower pre-emphasis, UI consistency, static filter mode, double-filtering warnings, sensible defaults, complex gain structure documentation
- **v6.0** (September 2025) - Added Earth and expanded BuzzBox conversion insights
- **v5.0** (September 2025) - Updated with Venus conversion lessons
- **v4.0** (August 2025) - Added Mars-specific optimizations
- **v3.0** (August 2025) - Initial structured guide

## Project Examples

All following projects use this methodology:
- **Mars Neural Amp**: Complex neural processing + IR
- **Venus Spectral Reverb**: Multi-algorithm reverb
- **Earth Reverbscape**: Octave + reverb + overdrive integration
- **BuzzBox Octa Squawker**: Fuzz + autowah + octave with context-dependent controls

Each demonstrates specific techniques and lessons incorporated into this guide.
