# Venus Hothouse - Spectral Reverb

Spectral reverb with shimmer, drift modulation, and detune capabilities. FFT-based frequency-domain processing for evolving ambient textures.

## Quick Start

### For Users (Pre-built Binary)
1. Download `venus_hothouse_v1.0.bin` from binary/ directory
2. Connect Hothouse via USB-C
3. Enter DFU mode: Hold BOOT, press/release RESET, release BOOT
4. Open [Daisy Web Programmer](https://electro-smith.github.io/Programmer/)
5. Load and flash the binary (Chrome/Edge only)

### For Developers (Build from Source)
```bash
cd funbox-to-hothouse-ports/venus-hothouse/src/
make clean
make
```

See `src/BUILD_INSTRUCTIONS.md` for detailed setup instructions.

## Hardware Controls

### Knobs
- **K1**: Decay (reverb time)
- **K2**: Mix (dry/wet balance)
- **K3**: Damp (high frequency damping)
- **K4**: Shimmer (octave generation amount)
- **K5**: Shimmer Tone (5th harmonic amount)
- **K6**: Detune (pitch shift ±15 cents)

### Toggle Switches
- **SW1**: Shimmer Mode
  - UP: Octave Down
  - CENTER: Octave Up
  - DOWN: Both Octaves
- **SW2**: Reverb Character
  - UP: Less Lofi
  - CENTER: Normal
  - DOWN: More Lofi
- **SW3**: Drift Speed
  - UP: Slow
  - CENTER: Off
  - DOWN: Fast

### Footswitches
- **FS1**: Bypass toggle
- **FS2**: Freeze (momentary - hold to sustain reverb tail)

### LEDs
- **LED1 (Red)**: Bypass indicator (off = processing, on = bypassed)
- **LED2 (Red)**: Freeze active indicator

## Features
- **Spectral Reverb**: 4096-point FFT with frequency-dependent decay
- **Shimmer Effects**: Octave up/down with independent 5th harmonics
- **Drift Modulation**: 4 independent LFOs for evolving textures
- **Detune**: ±15 cent pitch shifting for chorus-like effects
- **Freeze**: Capture and hold reverb tail
- **Lofi Modes**: Sample rate reduction (0.2-0.3x) with filtering

## Technical Details
- **Platform**: Cleveland Music Co. Hothouse (Daisy Seed)
- **Sample Rate**: 32 kHz (optimized for FFT)
- **Audio Block Size**: 256 samples
- **FFT Order**: 12 (4096 points, 4x overlap)
- **Processing**: Mono input, stereo output
- **CPU Usage**: ~85-90%
- **Memory**: SRAM mode (~100KB flash, ~50KB RAM)
- **Latency**: ~85ms round-trip

## Attribution
**Original Project**: Venus Spectral Reverb for Funbox  
**Original Author**: Keith Bloemer (GuitarML)  
**Original Repository**: https://github.com/GuitarML/FunBox/tree/main/software/Venus  
**Hothouse Port**: Chris Brandt (chris@futr.tv)  
**Port Date**: September 17, 2025

## License
MIT License - See LICENSE file in src/ directory for details.

## Documentation
- **Complete Controls Reference**: `src/CONTROLS_REFERENCE.md`
- **Build Instructions**: `src/BUILD_INSTRUCTIONS.md`
- **Source Code Documentation**: `src/README.md`
