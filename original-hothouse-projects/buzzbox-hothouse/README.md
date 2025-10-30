# BuzzBox Hothouse - Octa Squawker

Versatile fuzz pedal with envelope-controlled autowah filter and octave generation. Features context-dependent controls and flexible effect routing.

## Quick Start

### For Users (Pre-built Binary)
1. Download `buzzbox_hothouse_v1.0.bin` from binary/ directory
2. Connect Hothouse via USB-C
3. Enter DFU mode: Hold BOOT, press/release RESET, release BOOT
4. Open [Daisy Web Programmer](https://electro-smith.github.io/Programmer/)
5. Load and flash the binary (Chrome/Edge only)

### For Developers (Build from Source)
```bash
cd original-hothouse-projects/buzzbox-hothouse/src/src/
make clean
make
```

**Note:** The build process generates numerous compiler warnings from the STM32 HAL drivers (deprecated volatile operations). These are harmless and do not affect functionality. The warnings originate from libDaisy's HAL driver dependencies, not from the BuzzBox source code itself.

See `src/docs/BUILD_INSTRUCTIONS.md` for detailed setup instructions.

## Hardware Controls

### Fixed Controls (Knobs 1-3)
- **K1**: Input Gain (0.5x to 2.0x)
- **K2**: Dry/Wet Mix
- **K3**: Output Level

### Context-Dependent Controls (Knobs 4-6)
Function changes based on **SW3** position:

**SW3 UP (Fuzz Mode):**
- **K4**: Fuzz Drive
- **K5**: Tone (100-1500Hz)
- **K6**: Gate Threshold

**SW3 MIDDLE (Autowah Mode):**
- **K4**: Filter Speed
- **K5**: Filter Range
- **K6**: Envelope Threshold

**SW3 DOWN (Octave Mode):**
- **K4**: Octave Up Level
- **K5**: Octave Down Level
- **K6**: Octave Mix

### Toggle Switches
- **SW1**: Autowah Placement
  - UP: Before fuzz
  - CENTER: After fuzz
  - DOWN: After everything
- **SW2**: Footswitch 2 Function
  - UP: Both autowah + octave
  - CENTER: Autowah only
  - DOWN: Octave only
- **SW3**: Knobs 4-6 Mode (see above)

### Footswitches
- **FS1**: Fuzz on/off
- **FS2**: Autowah/Octave on/off (per SW2 setting)

### LEDs
- **LED1 (Red)**: Fuzz active indicator
- **LED2 (Red)**: Autowah/Octave active indicator

## Features
- **Vintage Fuzz**: Classic circuit with bass boost, variable drive, and tone (100-1500Hz)
- **Envelope Autowah**: Dynamic bandpass filter with speed, threshold, and range controls
- **Octave Generation**: Independent up/down octave synthesis with mixing
- **Touch-to-Activate**: Parameters hold values after mode switch until knob is moved
- **Flexible Routing**: Three autowah placement options
- **Automatic Gain Compensation**: Balanced output across all effect combinations
- **4x Oversampling**: On fuzz stage for reduced aliasing

## Technical Details
- **Platform**: Cleveland Music Co. Hothouse (Daisy Seed)
- **Sample Rate**: 48 kHz
- **Audio Block Size**: 256 samples
- **Octave Processing**: Multirate at 8kHz (6x decimation)
- **CPU Usage**: ~60-70% at 480MHz
- **Memory**: QSPI boot mode (~200KB SRAM used)
- **Latency**: ~5.3ms

## Signal Flow
1. Input Gain
2. Autowah (if SW1 = UP)
3. Octave Generation (multirate: 8kHz)
4. Fuzz (bass boost → gain → clipping → tone)
5. Autowah (if SW1 = MIDDLE)
6. Autowah (if SW1 = DOWN)
7. FS2 Gain Compensation (2x when effects active but fuzz off)
8. Dry/Wet Mix
9. Output Level

## Key Implementation Details

### Touch-to-Activate System
Prevents parameter jumps when switching modes. After mode change, parameters maintain their last value until the physical knob is moved, ensuring smooth transitions.

### Gain Compensation
Automatic level matching compensates for:
- Fuzz gain staging
- Octave generation output
- Autowah bandpass volume reduction (2x makeup gain)
- FS2 compensation (2x gain when effects active but fuzz bypassed)

### Multirate Processing
Octave generation uses 6x decimation to 8kHz for accurate pitch tracking while maintaining efficiency.

## Attribution
**Original Design**: Chris Brandt (chris@futr.tv)  
**Creation Date**: October 9, 2025

**Inspiration & Components:**
- Keith Bloemer (GuitarML) - Earth Reverbscape inspiration
- Cycfi Q DSP Library (Joel de Guzman) - Octave generation components
- Steve Schulteis - Octave optimization suggestions

**Third-Party Libraries**: DaisySP, libDaisy, Q DSP Library, Hothouse Library

## License
MIT License - See LICENSE file in src/ directory for details.

## Documentation
- **Build Instructions**: `src/docs/BUILD_INSTRUCTIONS.md`
- **Controls Reference**: `src/docs/CONTROLS_REFERENCE.md`
- **Signal Chain**: `src/docs/SIGNAL_CHAIN.md`
- **Source Code Documentation**: `src/README.md`
- **Attribution**: `src/ATTRIBUTION.md`
