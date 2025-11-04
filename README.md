# Daisy Seed Pedals

A collection of guitar effects pedals for the Daisy Seed platform, targeting the Cleveland Music Co. Hothouse hardware.

## Projects

### Funbox to Hothouse Ports
Original projects from GuitarML Funbox, ported to Hothouse:
- **[Mars](funbox-to-hothouse-ports/mars-hothouse/)** - Neural amp modeling with IR cabinets
- **[Venus](funbox-to-hothouse-ports/venus-hothouse/)** - Spectral reverb
- **[Earth](funbox-to-hothouse-ports/earth-hothouse/)** - Reverbscape with octave

### Original Hothouse Projects
- **[BuzzBox](original-hothouse-projects/buzzbox-hothouse/)** - Aggressive fuzz with envelope filter and octave
- **[FLUX](original-hothouse-projects/flux-hothouse/)** - Modulated slicer delay (in development)

## 🚀 Starter Kit for Building Your Own Pedals

New to Hothouse development? Want to build your own pedals with AI assistance?

Check out the **[Starter Kit](starter-kit/)** - a comprehensive package for building Hothouse pedals using Claude AI:

- 📝 **Quick Start Prompt** - Copy/paste and start building immediately
- 🎯 **Session Templates** - Structured development with Claude Projects  
- 📚 **API Reference** - Complete Hothouse platform documentation
- ⚠️ **Gotcha Database** - Avoid 100+ common pitfalls
- 📖 **Examples** - From simple tremolo to complex multi-effects

**Based on real production pedals**: Venus, Mars, Earth, BuzzBox, and FLUX

👉 **[Get Started →](starter-kit/README.md)**

## Quick Start

### Prerequisites
- Daisy Seed compatible hardware (Hothouse)
- arm-none-eabi-gcc toolchain
- dfu-util for flashing
- Git with submodules support

### Clone with Submodules
```bash
git clone --recurse-submodules https://github.com/cbcbcbd/daisy-seed-pedals.git
cd daisy-seed-pedals
```

### Build a Project
```bash
cd funbox-to-hothouse-ports/mars-hothouse
make clean
make
```

### Flash to Hardware
```bash
# First time only - install bootloader
make program-boot

# Regular flashing via DFU
make program-dfu
```

## Documentation
- [Building Projects](docs/development/Building_Projects.md)
- [Hardware Reference](docs/hardware/Hothouse_Platform_Reference.md)
- [Conversion Guide](docs/conversion/Funbox_to_Hothouse_Guide.md)

## Repository Structure
```
daisy-seed-pedals/
├── funbox-to-hothouse-ports/    # Ported projects
├── original-hothouse-projects/  # Original designs
├── tools/                       # Development tools
├── docs/                        # Documentation
├── libDaisy/                    # Submodule
└── DaisySP/                     # Submodule
```

## Hardware

### Currently Supported Platforms
All current projects target the **Cleveland Music Co. Hothouse** platform:
- 6 knobs
- 3 toggle switches
- 2 footswitches
- 2 LEDs (red)
- Stereo I/O
- Based on Daisy Seed

Additional platforms may be added in the future.

## License
MIT License - See individual project directories for attribution details.

## Author
Chris Brandt (chris@futr.tv)
