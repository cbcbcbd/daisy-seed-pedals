# Mars Hothouse - Neural Amp Modeler

Neural network amp modeling with impulse response cabinet simulation and delay effects. Real-time neural inference using RTNeural library.

## Quick Start

### For Users (Pre-built Binary)
1. Download `mars_hothouse_v1.1.bin` from binary/ directory
2. Connect Hothouse via USB-C
3. Enter DFU mode: Hold BOOT, press/release RESET, release BOOT
4. Open [Daisy Web Programmer](https://electro-smith.github.io/Programmer/)
5. Load and flash the binary (Chrome/Edge only)

### For Developers (Build from Source)
```bash
cd funbox-to-hothouse-ports/mars-hothouse/src/
make clean
make
```

See `src/BUILD_INSTRUCTIONS.md` for detailed setup instructions.

## Hardware Controls

### Knobs
- **K1**: Input Gain (0.1-2.5x)
- **K2**: Dry/Wet Mix
- **K3**: Output Level
- **K4**: Tone (LP/HP filter)
- **K5**: Delay Time (50ms-1s)
- **K6**: Delay Feedback

### Toggle Switches
- **SW1**: Amp Model
  - UP: Model 1
  - CENTER: Model 2
  - DOWN: Model 3
- **SW2**: Cabinet IR
  - UP: IR 1
  - CENTER: IR 2
  - DOWN: IR 3
- **SW3**: Delay Mode
  - UP: Normal
  - CENTER: Dotted Eighth
  - DOWN: Triplet

### Footswitches
- **FS1**: Bypass toggle (long press for DFU mode)
- **FS2**: Delay on/off

### LEDs
- **LED1 (Red)**: Bypass indicator (off = processing, on = bypassed)
- **LED2 (Red)**: Delay active indicator

## Features
- **Neural Amp Modeling**: 3 pre-trained GRU-based amp models
- **Cabinet IRs**: 3 impulse responses for speaker simulation
- **Delay Effects**: Normal, dotted eighth, and triplet modes
- **Tone Control**: Combined low-pass/high-pass filter
- **Real-time Inference**: Optimized neural network processing

## Technical Details
- **Platform**: Cleveland Music Co. Hothouse (Daisy Seed)
- **Sample Rate**: 48 kHz
- **Audio Block Size**: 256 samples
- **Neural Network**: GRU-9 architecture with 4 model instances
- **IR Processing**: Convolution-based cabinet simulation
- **CPU Usage**: ~90% (neural + IR + delay)
- **Memory**: QSPI boot mode with SDRAM delay buffer
- **Delay Buffer**: 48k samples (1 second maximum)

## Version History
- **v1.1** (September 23, 2025)
  - Added Footswitch 2 delay enable/disable
  - Optimized delay range (50ms-1s)
  - Added LED 2 delay status indicator
  - Fixed IR processing
  - Restored original amp model values

- **v1.0** - Initial Hothouse port

## Attribution
**Original Project**: Mars Neural Amp for Funbox  
**Original Author**: Keith Bloemer (GuitarML)  
**Original Repository**: https://github.com/GuitarML/FunBox/tree/main/software/Mars  
**Hothouse Port**: Chris Brandt (chris@futr.tv)  
**v1.1 Enhancements**: September 23, 2025

**Neural Network**: RTNeural library  
**Impulse Responses**: Included cabinet simulations

## License
MIT License - See LICENSE file in src/ directory for details.

## Documentation
- **Build Instructions**: `src/BUILD_INSTRUCTIONS.md`
- **Changelog**: `src/CHANGELOG.md`
- **Dependencies**: `src/DEPENDENCIES.md`
- **Source Code Documentation**: `src/README.md`
