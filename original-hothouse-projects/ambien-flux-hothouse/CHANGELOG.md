# Changelog

All notable changes to Ambien Flux will be documented in this file.

## [1.0] - 2024-11-03

### Initial Release
Complete implementation of Sample & Hold slicer delay pedal.

#### Features
- **Core Slicing Engine**
  - 16 discrete slice buffers (500ms max per slice)
  - Variable slice count (1-16)
  - Adjustable slice length (100-500ms logarithmic)
  - Zero-crossing detection for click-free transitions
  - Variable crossfade system (15% proportional, 5ms minimum)

- **Playback Modes**
  - Forward/Forward: Normal capture and playback
  - Reverse: Backward sequence with reverse playback
  - Random: Forward capture with randomized direction per slice

- **Stutter System**
  - Shuffle probability (0-100%)
  - Musical subdivisions (1x, 2x, 4x, 8x)
  - Random slice selection

- **Lo-Fi Effects**
  - Bit crushing with 50% Nyquist low-pass filter
  - Wobble/flutter (0.5-6Hz LFO-modulated delay)
  - Dust (sparse vinyl crackle with 600Hz low-pass)

- **Freeze Mode**
  - Latching freeze toggle
  - Stops capture, continues playback
  - All controls remain active while frozen

- **User Interface**
  - Two-way toggle mode selector (Normal/Lo-Fi)
  - Touch detection for K3-K6 parameters
  - LED indicators for effect/freeze status

#### Technical
- Platform: Cleveland Music Co. Hothouse (Daisy Seed)
- Sample Rate: 48 kHz
- Processing: 32-bit float
- CPU: STM32H750 @ 480MHz
- Latency: <5ms

#### Development Notes
- Envelope follower system implemented but commented out
- Reserved Toggle 2 and Toggle 3 MIDDLE for future expansion
- Source code includes detailed signal flow documentation
