# Hothouse Pedal Development - Quick Start Prompt

**Copy and paste this entire document at the start of any Claude session for Hothouse pedal development.**

---

## I'm developing an audio effects pedal for the Cleveland Music Co. Hothouse platform

### Platform Overview
- **Hardware**: Daisy Seed (STM32H750, 480MHz) on Hothouse board
- **Controls**: 6 knobs, 3 toggle switches (3-position), 2 footswitches, 2 red LEDs
- **Audio**: Mono in/out, 48kHz sample rate, 32-bit float processing
- **Namespace**: `clevelandmusicco`

### Critical Hothouse API Reference

**Required Files** (never recreate, always copy from working repository):
```cpp
hothouse.cpp  // Must be in CPP_SOURCES in Makefile
hothouse.h    // Header file
```

**Required Includes**:
```cpp
#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"
using namespace clevelandmusicco;
```

**Hardware Object**:
```cpp
Hothouse hw;
hw.Init(true);  // true = 480MHz boost, false = 400MHz default
```

**Control Processing** (call once per audio callback):
```cpp
hw.ProcessAllControls();  // Single method handles all controls
```

**Reading Controls**:
```cpp
// Knobs (returns 0.0 to 1.0)
float k1 = hw.GetKnobValue(Hothouse::KNOB_1);
float k2 = hw.GetKnobValue(Hothouse::KNOB_2);
float k3 = hw.GetKnobValue(Hothouse::KNOB_3);
float k4 = hw.GetKnobValue(Hothouse::KNOB_4);
float k5 = hw.GetKnobValue(Hothouse::KNOB_5);
float k6 = hw.GetKnobValue(Hothouse::KNOB_6);

// Toggle Switches (returns 0=UP, 1=CENTER, 2=DOWN)
// CRITICAL: Physical position is INVERTED from software value
// Physical DOWN = case 2, Physical UP = case 0
int toggle1 = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
int toggle2 = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2);
int toggle3 = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3);

// Footswitches
if(hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
    // Button pressed
}
if(hw.switches[Hothouse::FOOTSWITCH_1].Pressed()) {
    // Button held
}
```

**LEDs** (both red on Hothouse):
```cpp
// In main(), after hw.Init()
led1.Init(hw.seed.GetPin(Hothouse::LED_1), false);
led2.Init(hw.seed.GetPin(Hothouse::LED_2), false);

// In audio callback or control processing
led1.Set(bypass ? 0.0f : 1.0f);
led2.Set(some_condition ? 1.0f : 0.0f);
led1.Update();
led2.Update();
```

### Critical "Gotchas" - Read These First!

1. **hothouse.cpp MUST be in Makefile**: Add to `CPP_SOURCES` line or linker fails
2. **Toggle switches are inverted**: Physical DOWN = software value 2, Physical UP = value 0
3. **Both LEDs are red**: Not red/green like some other platforms
4. **ProcessAllControls() is required**: Single method replaces separate analog/digital calls
5. **Never modify bootloader code**: Copy exact bootloader entry from working projects
6. **Clear buffers when effects are off**: Prevents noise artifacts
7. **Multirate buffer alignment**: Increment indices AFTER use, not before
8. **CPU limits are real**: Neural networks + complex DSP can overload at 480MHz
9. **fonepole() parameter order**: `fonepole(target, input, coefficient)` - target is float&
10. **Always test on hardware**: Simulation doesn't catch timing/CPU issues

### DaisySP Common Patterns

**Smoothing parameters** (prevent zipper noise):
```cpp
float smooth_value;
fonepole(smooth_value, knob_value, 0.0002f);  // Note: first param is reference
```

**Oscillators**:
```cpp
Oscillator osc;
osc.Init(sample_rate);
osc.SetWaveform(Oscillator::WAVE_SIN);
osc.SetFreq(440.0f);
osc.SetAmp(0.5f);
float sample = osc.Process();
```

**Filters**:
```cpp
Tone tone;
tone.Init(sample_rate);
tone.SetFreq(1000.0f);
float filtered = tone.Process(input);
```

**Effects**:
```cpp
Overdrive overdrive;
overdrive.Init();
overdrive.SetDrive(0.5f);
float processed = overdrive.Process(input);
```

### Typical Project Structure

```cpp
#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"

using namespace daisy;
using namespace daisysp;
using namespace clevelandmusicco;

Hothouse hw;
Led led1, led2;

// DSP objects
// [Your effects modules here]

// Control values
float knobValues[6];
int toggleValues[3];
bool bypass = false;

void ProcessControls() {
    hw.ProcessAllControls();
    
    // Read all knobs
    for(int i = 0; i < 6; i++) {
        knobValues[i] = hw.GetKnobValue(static_cast<Hothouse::Knob>(i));
    }
    
    // Read toggles
    for(int i = 0; i < 3; i++) {
        toggleValues[i] = hw.GetToggleswitchPosition(static_cast<Hothouse::Toggleswitch>(i));
    }
    
    // Footswitch for bypass
    if(hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }
    
    // Update LEDs
    led1.Set(bypass ? 0.0f : 1.0f);
    led1.Update();
    led2.Update();
}

void AudioCallback(AudioHandle::InputBuffer in,
                   AudioHandle::OutputBuffer out,
                   size_t size) {
    ProcessControls();
    
    for(size_t i = 0; i < size; i++) {
        float input = in[0][i];
        float output;
        
        if(bypass) {
            output = input;
        } else {
            // [Your DSP processing here]
            output = input;
        }
        
        out[0][i] = out[1][i] = output;
    }
}

int main(void) {
    hw.Init(true);  // 480MHz boost
    hw.SetAudioBlockSize(4);
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    
    // Initialize LEDs
    led1.Init(hw.seed.GetPin(Hothouse::LED_1), false);
    led2.Init(hw.seed.GetPin(Hothouse::LED_2), false);
    
    // Initialize DSP objects
    // [Your initialization here]
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    // Main loop for bootloader entry (copy exact code from working project)
    while(1) {
        // Bootloader entry code here
    }
}
```

### Development Workflow

**Mode: GUIDED** - Always ask before implementing anything

1. **Start with requirements**: What does the pedal do? What are the controls?
2. **Design signal flow**: Document the complete audio path
3. **Implement incrementally**: Basic structure → controls → DSP → testing
4. **Test on hardware**: Don't rely on simulation alone
5. **Document everything**: Control mappings, parameter ranges, design decisions

### Building and Flashing

```bash
# Clean build
make clean
make

# Flash via DFU
# 1. Hold BOOT, press and release RESET, release BOOT
# 2. Use web programmer: https://electro-smith.github.io/Programmer/
# OR command line:
make program-dfu
```

### When Converting from Funbox

**API Translation**:
- `hw.ProcessAnalogControls()` + `hw.ProcessDigitalControls()` → `hw.ProcessAllControls()`
- `hw.knob[KNOB_1].Process()` → `hw.GetKnobValue(Hothouse::KNOB_1)`
- `hw.toggle[0].Pressed()` → `hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)`
- Hardware object: `DaisyPetal hw;` → `Hothouse hw;`
- Namespace: `using namespace funbox;` → `using namespace clevelandmusicco;`

**Preserve original DSP exactly**: Don't "fix" things that look wrong - test first!

---

## Request Format

When asking me to help with Hothouse development:

**For new pedals**: "I want to create a [effect type] pedal with [description of features]"

**For conversions**: "I'm converting [project name] from Funbox to Hothouse. Here's the original code: [paste code]"

**For debugging**: "I'm getting [specific issue]. Here's the relevant code: [paste code section]"

**For implementation**: "Please implement [specific feature] following GUIDED mode (ask before coding)"

---

## Mode Behavior

**GUIDED MODE** (default for development):
- ASK before implementing anything
- Present plan and wait for approval
- Report token usage regularly
- Implement incrementally with testing
- Never assume requirements

Start every response with token status: `Tokens: X used | Y remaining | Z% remaining (ZONE) 🟢`

---

## Ready to Start

I'm ready to help you build a Hothouse pedal. What would you like to create?
