# Ambien Flux

**Sample & Hold Slicer Delay for Cleveland Music Co. Hothouse**

Transform your guitar into a glitch machine. Ambien Flux captures discrete audio slices and plays them back with controllable repetition, randomization, and lo-fi processing. Freeze moments in time and manipulate them rhythmically.

## Features

### Core Slicing Engine
- **Sample & Hold Architecture**: Captures audio into 16 discrete slice buffers
- **Variable Slice Count**: 1-16 slices per cycle
- **Adjustable Slice Length**: 100-500ms with logarithmic curve
- **Zero-Crossing Detection**: Click-free slice transitions
- **Variable Crossfade**: Smooth slice boundaries (15% proportional, 5ms minimum)

### Playback Modes
- **Forward/Forward**: Normal capture and playback
- **Reverse**: Backward slice sequence with reverse playback
- **Random**: Forward capture with randomized playback direction per slice

### Stutter System
- **Shuffle Probability**: 0-100% chance of slice repetition
- **Musical Subdivisions**: 1x, 2x, 4x, or 8x repeats
- **Dynamic Glitching**: Random slice selection for unpredictable textures

### Lo-Fi Effects
- **Bit Crushing**: Sample rate reduction with aggressive 50% Nyquist low-pass filter
- **Wobble/Flutter**: LFO-modulated delay (0.5-6Hz) for tape wow/uni-vibe character
- **Dust**: Sparse vinyl crackle (0-2% density, 600Hz low-pass, progressive mix)

### Freeze Mode
- **Latching Freeze**: Stop capturing, loop current buffer
- **Live Control**: Adjust feedback, mix, and effects while frozen
- **Layer Building**: Stack patterns with feedback on frozen content

## Controls

### Knobs (Always Active)
- **K1**: MASTER LEVEL (0-200% / 0dB to +6dB)
- **K2**: DRY/WET MIX (0-100%)

### Toggle 3 UP - Normal Mode
- **K3**: FEEDBACK (0-100% - pattern regeneration)
- **K4**: SLICE COUNT (1-16 slices per cycle)
- **K5**: SLICE LENGTH (100-500ms)
- **K6**: STUTTER (0-100% - random repetition probability)

### Toggle 3 DOWN - Lo-Fi Mode
- **K3**: WOBBLE (0-100% - tape wow/flutter intensity)
- **K4**: DUST (0-100% - vinyl crackle density)
- **K5**: BIT CRUSH (0-100% - sample rate reduction)
- **K6**: (Unused)

### Toggle 1 - Playback Modes
- **UP**: Forward capture → Forward playback
- **MIDDLE**: Backward capture → Reverse playback
- **DOWN**: Forward capture → Random playback direction

### Footswitches
- **FS1 TAP**: Toggle bypass on/off
- **FS1 HOLD** (2 seconds): Enter bootloader for firmware updates
- **FS2 TAP**: Toggle freeze (stops capture, loops current buffer)

### LEDs
- **LED1**: Effect active (on when not bypassed)
- **LED2**: Freeze active (on when frozen)

## Signal Flow

```
INPUT
  ↓
[Bit Crush] ← Applied before capture (vintage sampler aesthetic)
  ↓
Slice Capture → 16 Discrete Buffers
  ↓
Slice Playback (Forward/Reverse/Random)
  ↓
[Dry/Wet Mix]
  ↓
[Wobble] ← Tape flutter on mixed signal
  ↓
[Dust] ← Vinyl crackle on top
  ↓
[Master Level]
  ↓
OUTPUT
```

## Installation

### Quick Start
1. Download the latest binary: `ambien_flux_v1.0.bin`
2. Connect Hothouse to computer via USB
3. Hold FS1 for 2 seconds to enter bootloader mode (LEDs flash)
4. Use [Daisy Web Programmer](https://electro-smith.github.io/Programmer/) to upload binary
5. Power cycle the pedal

### Building from Source
Requires libDaisy and DaisySP libraries installed as submodules.

```bash
cd ambien-flux-hothouse
make clean
make
# Binary output: build/ambien_flux.bin
```

## Sound Design Tips

### Rhythmic Slicing
- Set slice count to match tempo divisions (4, 8, 16)
- Use lower feedback for tight rhythms
- Add stutter for syncopation

### Ambient Textures
- Longer slice lengths (400-500ms)
- Higher feedback (60-80%)
- Reverse mode for ethereal sounds
- Freeze and layer with feedback

### Glitch Effects
- Short slices (100-150ms) with high stutter
- Random playback mode
- Lo-fi mode with bit crushing and wobble
- Freeze and manipulate frozen content

### Lo-Fi Character
- Bit crush before slicing for vintage sampler sound
- Wobble adds tape/uni-vibe warble
- Dust creates vinyl texture
- Combine all three for maximum degradation

## Technical Specifications

- **Platform**: Cleveland Music Co. Hothouse (Electro-Smith Daisy Seed)
- **Processor**: STM32H750 @ 480MHz
- **Sample Rate**: 48 kHz
- **Bit Depth**: 32-bit float internal processing
- **Buffer Memory**: 16 slices × 24,000 samples (500ms max per slice)
- **Latency**: <5ms (buffer-dependent)
- **Power**: USB-C or 9V DC center-negative

## Credits

- **Design & Development**: Chris Brandt
- **Platform**: Cleveland Music Co. Hothouse
- **DSP Library**: Electro-Smith DaisySP
- **Development Tool**: Claude.ai (Anthropic)

## License

MIT License - See LICENSE file for details

## Version History

### v1.0 (November 2024)
- Initial release
- Core slicing engine with 16 buffers
- Three playback modes (Forward/Reverse/Random)
- Stutter system with musical subdivisions
- Lo-fi effects (Bit crush, Wobble, Dust)
- Freeze mode
- Touch detection for parameter switching

## Support

For issues, questions, or feature requests, please visit the GitHub repository.

---

**Ambien Flux** - Where time bends and moments repeat.
