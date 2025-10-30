# Earth Hothouse - Reverbscape

Sophisticated multi-effect combining Dattorro plate reverb, multirate octave generation, and overdrive with flexible routing and momentary controls.

## Quick Start

### For Users (Pre-built Binary)
1. Download `earth_hothouse_v1.0.bin` from binary/ directory
2. Connect Hothouse via USB-C
3. Enter DFU mode: Hold BOOT, press/release RESET, release BOOT
4. Open [Daisy Web Programmer](https://electro-smith.github.io/Programmer/)
5. Load and flash the binary (Chrome/Edge only)

### For Developers (Build from Source)
```bash
cd funbox-to-hothouse-ports/earth-hothouse/src/
make clean
make
```

**Note:** The build process generates numerous compiler warnings from the STM32 HAL drivers (deprecated volatile operations). These are harmless and do not affect functionality. The warnings originate from libDaisy's HAL driver dependencies, not from the Earth source code itself.

See `src/BUILD_INSTRUCTIONS.md` for detailed setup instructions.

## Hardware Controls

### Knobs
- **K1**: Reverb Pre-delay (0-100ms)
- **K2**: Dry/Wet Mix
- **K3**: Reverb Decay (short to infinite)
- **K4**: Modulation Depth
- **K5**: Modulation Speed
- **K6**: Damping (reverb brightness)

### Toggle Switches
- **SW1**: Reverb Time Scale
  - UP: Normal (1.0x)
  - CENTER: Extended (2.0x)
  - DOWN: Long (4.0x)
- **SW2**: Effect Mode
  - UP: Reverb only
  - CENTER: Reverb + Octave up
  - DOWN: Reverb + All octaves (up/down/sub)
- **SW3**: Footswitch 2 Function
  - UP: Freeze (infinite reverb hold)
  - CENTER: Overdrive boost
  - DOWN: Momentary effect toggle

### Footswitches
- **FS1**: Bypass toggle (long press for DFU mode)
- **FS2**: Momentary function (based on SW3 setting)

### LEDs
- **LED1 (Red)**: Bypass indicator (off = processing, on = bypassed)
- **LED2 (Red)**: Footswitch 2 function active

## Features
- **Dattorro Plate Reverb**: Classic algorithm with input diffusion and tank modulation
- **Multirate Octave Generation**: Sub-octave, octave down, and octave up with intelligent pitch tracking
- **Overdrive Stage**: Smooth drive with automatic volume compensation
- **Freeze Function**: Infinite reverb hold with wet signal reduction
- **Flexible Routing**: Multiple effect combinations and momentary control modes
- **Expression Pedal Support**: MIDI-based expression control

## Technical Details
- **Platform**: Cleveland Music Co. Hothouse (Daisy Seed)
- **Sample Rate**: 48 kHz
- **Audio Block Size**: 48 samples
- **Octave Processing**: 6x decimation for pitch tracking
- **CPU Usage**: ~75-80%
- **Memory**: SRAM mode (~116KB flash)
- **Latency**: ~1ms base latency

## Key Implementation Details

### Multirate Processing
The octave generator processes every 6th sample at reduced rate for accurate pitch tracking. Buffer management maintains separate input (`buff`) and output (`buff_out`) buffers synchronized with the bin counter.

### Volume Compensation
The overdrive uses an artistic formula for volume compensation: `1.0f - (current_ODswell * current_ODswell * 2.8f - 0.1296f)`. This creates an intentional bloom/fade effect as overdrive increases.

### Freeze Implementation
When freeze is active, reverb decay ramps to 1.0 (infinite) and wet output is reduced by 40% to prevent excessive build-up while maintaining the frozen tail.

## Attribution
**Original Project**: Earth Reverbscape for Funbox  
**Original Author**: Keith Bloemer (GuitarML)  
**Original Repository**: https://github.com/GuitarML/FunBox/tree/main/software/Earth  
**Hothouse Port**: Chris Brandt (chris@futr.tv)  
**Port Date**: September 5, 2025

**Reverb Algorithm**: Dattorro plate reverb (Mooer A7 ambience)  
**DSP Libraries**: Q DSP library, gcem compile-time math

## License
MIT License - See LICENSE file in src/ directory for details.

## Documentation
- **Complete Controls Reference**: `src/CONTROLS_REFERENCE.md`
- **Build Instructions**: `src/BUILD_INSTRUCTIONS.md`
- **Source Code Documentation**: `src/README.md`
