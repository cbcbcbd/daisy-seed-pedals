# Hothouse Pedal Development Starter Package

**Build professional audio effect pedals for the Cleveland Music Co. Hothouse platform using Claude AI**

---

## What's This?

This package contains everything you need to develop guitar/audio effect pedals for the Hothouse platform using Claude as your AI development assistant. It includes comprehensive API references, proven patterns, and templates that guide Claude to help you build, convert, and debug pedal code effectively.

### What is Hothouse?

The **Cleveland Music Co. Hothouse** is a guitar pedal development platform based on the Electro-Smith Daisy Seed microcontroller. It features:
- 6 potentiometers (knobs)
- 3 three-position toggle switches
- 2 footswitches
- 2 LEDs
- Professional audio I/O
- 480MHz ARM Cortex-M7 processor

Perfect for creating custom effects pedals, converting existing designs, or learning DSP programming.

---

## Package Contents

| File | Description | Size | Use Case |
|------|-------------|------|----------|
| **HOTHOUSE_QUICK_START_PROMPT.md** | Complete standalone reference | ~3K tokens | Quick start, no setup |
| **SESSION_STARTER_TEMPLATE.md** | Structured session template | ~1.5K tokens | Claude Projects |
| **HOTHOUSE_ESSENTIALS.json** | Machine-readable API reference | ~2K tokens | Knowledge base |
| **SETUP_INSTRUCTIONS.md** | Complete setup guide | Reference | Getting started |
| **README.md** | This file | Reference | Overview |

---

## Quick Start (3 Minutes)

### Simplest Way: Copy & Paste

1. Open a new conversation with Claude
2. Copy **HOTHOUSE_QUICK_START_PROMPT.md** entire contents
3. Paste into Claude
4. Add: "I want to create a [delay/reverb/fuzz/etc.] pedal"
5. Start developing!

### Best Way: Claude Project (5 Minutes)

1. Create a new Claude Project named "Hothouse Development"
2. Upload **HOTHOUSE_ESSENTIALS.json** to project knowledge
3. Upload **SESSION_STARTER_TEMPLATE.md** to project knowledge
4. Start new chat, paste **SESSION_STARTER_TEMPLATE.md**
5. Customize the SESSION INFO section
6. Start developing!

**See SETUP_INSTRUCTIONS.md for detailed setup guide**

---

## What Can You Build?

### Create Original Pedals
- Delays (analog, digital, tape echo, multi-tap)
- Reverbs (hall, plate, room, shimmer, reverse)
- Modulation (chorus, flanger, phaser, tremolo, vibrato)
- Distortion/Overdrive/Fuzz
- Filters (wah, auto-wah, envelope filter, resonant)
- Pitch effects (octave, harmony, pitch shift)
- Loopers and sample-based effects
- Multi-effects combinations

### Convert Existing Pedals
- Port Funbox pedals to Hothouse
- Adapt open-source designs
- Clone classic pedals
- Update and improve existing code

### Learn DSP Programming
- Understand audio processing algorithms
- Experiment with DSP modules
- Build progressively complex effects
- Debug and optimize code

---

## Key Features

### Intelligent Mode System
Claude adapts behavior based on development stage:
- **GUIDED**: Ask before implementing (default for development)
- **EXPANSION**: Full requirements gathering before coding
- **REPLICATION**: Exact conversions, preserve all quirks
- **DEBUG**: Fix only what's broken
- **OPTIMIZATION**: Careful performance improvements

### Comprehensive Gotcha Database
Avoid common pitfalls:
- Toggle switch position inversions
- CPU usage limits
- Buffer management
- Multirate processing
- And 100+ more learned from real projects

### Proven Patterns
Based on successful conversions of Venus, Mars, Earth, and BuzzBox pedals:
- Hardware interface setup
- DSP signal flow
- Control processing order
- LED management
- Bootloader entry

### Token Management
Built-in token awareness:
- 🟢 GREEN: Normal development
- 🟡 YELLOW: Concise mode
- 🟠 ORANGE: Wrap-up
- 🔴 RED: Emergency handoff

---

## Example Projects

### Simple: Tremolo Pedal
```
Mode: EXPANSION
Controls:
- K1: Rate (0.1-10 Hz)
- K2: Depth (0-100%)
- K3: Waveform (sine, square, triangle)
- Toggle 1: Stereo mode
- FS1: Bypass
```

### Medium: Delay with Modulation
```
Mode: EXPANSION  
Controls:
- K1: Time (50ms-2s)
- K2: Feedback (0-95%)
- K3: Mix
- K4: Modulation depth
- K5: Modulation rate
- Toggle 1: Delay type (clean, tape, lo-fi)
- FS1: Bypass
- FS2: Tap tempo
```

### Advanced: Multi-Mode Reverb
```
Mode: EXPANSION
Controls:
- K1: Decay time
- K2: Pre-delay
- K3: Mix
- K4: Tone (filter)
- K5: Modulation
- K6: Shimmer amount
- Toggle 1: Algorithm (Hall, Plate, Room)
- Toggle 2: Shimmer mode
- Toggle 3: Effect routing
- FS1: Bypass
- FS2: Freeze
```

---

## Requirements

### Hardware Required
- Cleveland Music Co. Hothouse board
- Electro-Smith Daisy Seed (included with Hothouse)
- USB-C cable for programming
- 9V DC power supply (center-negative)
- Guitar/audio cables for testing

### Software Required
- ARM GCC toolchain
- Make
- DFU-util (for programming)
- Chrome or Edge browser (for web programmer)
- Git (for libDaisy/DaisySP)

### Claude Requirements
- Claude.ai account (Free or Pro)
- For best results: Claude Pro (Projects feature)

### Knowledge Required
- Basic C/C++ understanding helpful but not required
- No DSP knowledge needed - Claude explains concepts
- Guitar pedal familiarity helpful for design decisions

---

## What Makes This Different?

### Versus Starting from Scratch
❌ Without package: Hours researching API, learning pitfalls, debugging common issues
✅ With package: Proven patterns, comprehensive gotcha database, guided development

### Versus Generic AI Coding
❌ Generic AI: Makes up APIs, suggests incorrect patterns, ignores platform limitations
✅ This package: Platform-specific knowledge, proven patterns, real-world testing

### Versus Traditional Documentation
❌ Traditional docs: Search, read, interpret, implement, debug
✅ This package: Conversational guidance, contextual help, incremental testing

---

## Success Stories

This package is based on real production pedals:

**Venus Spectral Reverb** (Funbox→Hothouse conversion)
- Complex multi-algorithm reverb
- MIDI control support
- 100% functional conversion

**Mars Neural Amp Modeler** (Funbox→Hothouse conversion)
- Neural network inference
- Octave processing
- Complex CPU optimization

**Earth Reverbscape** (Funbox→Hothouse conversion)
- Multi-mode reverb/delay
- Freeze functionality
- Preset system

**BuzzBox Fuzz** (Original Hothouse design)
- Vintage-style fuzz circuit
- Auto-wah filter
- Octave-up effect
- Complex mode switching

All used these exact patterns and knowledge.

---

## Common Questions

**Q: Do I need to know DSP programming?**
A: No! Claude explains concepts as you go. This package includes proven DSP patterns.

**Q: Can I use this with Claude Free tier?**
A: Yes! Quick Start method works fine. Projects feature requires Pro.

**Q: Will my code be production-ready?**
A: Code follows professional patterns and is tested on hardware. You should always test thoroughly.

**Q: Can I convert pedals from other platforms?**
A: Yes! Package includes comprehensive Funbox→Hothouse conversion guide.

**Q: How long does it take to build a pedal?**
A: Simple effects: 1-2 hours. Complex multi-mode: 4-8 hours. Conversions: 2-6 hours.

**Q: What if I get stuck?**
A: Package includes debugging patterns. Claude can help troubleshoot with proper context.

**Q: Can I modify the templates?**
A: Absolutely! Customize for your workflow. Share improvements!

---

## Token Budget Guidelines

Claude sessions have token limits. Here's how to plan:

**Simple pedal (delay, tremolo)**: 30-50K tokens (1-2 sessions)
**Medium complexity (multi-mode reverb)**: 50-100K tokens (2-3 sessions)
**Complex (neural network, multi-fx)**: 100-150K tokens (3-5 sessions)
**Funbox conversion**: 40-80K tokens (2-3 sessions)

Each session = 190K tokens on Claude Pro

---

## Development Workflow

```
1. Requirements → 2. Design → 3. Implement → 4. Test → 5. Refine
     ↓              ↓            ↓           ↓          ↓
  EXPANSION     GUIDED       GUIDED      DEBUG    OPTIMIZATION
```

Typical session progression:
1. **EXPANSION**: Gather all requirements, design control layout
2. **GUIDED**: Implement hardware interface, add DSP modules
3. **DEBUG**: Fix issues found during testing
4. **OPTIMIZATION**: Improve performance, refine features
5. **Repeat** for additional features

---

## Best Practices

### DO ✅
- Specify Mode at session start
- Test on hardware after each feature
- Use REPLICATION for conversions
- Save working versions frequently
- Ask Claude to explain unfamiliar concepts
- Report token status concerns

### DON'T ❌
- Skip session starter template
- Let Claude code without approval (in GUIDED)
- Test only in simulation
- Add multiple complex features at once
- Modify bootloader without checking
- Forget hothouse.cpp in Makefile

---

## Community and Support

**Official Resources**:
- Hothouse: https://github.com/clevelandmusicco/HothouseExamples
- Daisy: https://electro-smith.github.io/libDaisy/
- Forum: https://forum.electro-smith.com/

**Package Maintainer**:
- Created by Chris Brandt based on production pedal development
- Tested on Venus, Mars, Earth, BuzzBox, and FLUX projects
- Continuously updated with new patterns and gotchas

**Contributing**:
- Found a new gotcha? Document it!
- Built something cool? Share the pattern!
- Improved the templates? Submit updates!

---

## What's Next?

### Getting Started Now
1. Read **SETUP_INSTRUCTIONS.md** for detailed setup
2. Choose Quick Start or Project approach
3. Grab hothouse.cpp/h from HothouseExamples repo
4. Start building!

### Going Deeper
- Study example projects in HothouseExamples
- Read DaisySP documentation for DSP modules
- Join the community forum
- Experiment with different algorithms

### Building Your Portfolio
- Start with simple effects (tremolo, boost)
- Progress to moderate complexity (delay, reverb)
- Eventually tackle advanced projects (multi-mode, AI)
- Share your creations with the community

---

## License

This starter package is provided as-is for educational and development purposes. 

The package templates and documentation are freely usable. Code you generate with Claude is yours to use however you like.

Hothouse hardware, libDaisy, and DaisySP have their own licenses - please respect them.

---

## Version

**Package Version**: 1.0
**Created**: November 2025
**Based on**: Real production pedal development (Venus, Mars, Earth, BuzzBox, FLUX)
**Platform**: Cleveland Music Co. Hothouse
**Tested with**: Claude Sonnet 3.5/4.0

---

## Ready to Build?

Choose your path:
- **Quick Start**: Open **HOTHOUSE_QUICK_START_PROMPT.md**
- **Full Setup**: Read **SETUP_INSTRUCTIONS.md**
- **Jump In**: Create project, upload files, start coding!

**Build amazing guitar pedals with AI assistance. Let's go!** 🎸🎛️🤖
