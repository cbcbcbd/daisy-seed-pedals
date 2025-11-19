# Q DSP Library Integration Notes

## Overview

Q DSP is a header-only C++ library for audio DSP, optimized for embedded systems.

**Repository:** https://github.com/cycfi/q  
**License:** MIT  
**Author:** Joel de Guzman (Cycfi Research)

## Integration Steps (When Ready)

1. **Clone Q DSP Library**
   ```bash
   cd ~/Desktop/+DEVELOPMENT+/daisy-seed-pedals/
   git clone https://github.com/cycfi/q.git q_lib
   ```

2. **Add to Makefile**
   ```makefile
   CPP_INCLUDES += -I../../q_lib/q_lib/include
   ```

3. **Copy BK Shepherd's Wrapper**
   - Copy `FrequencyDetectorQ.cpp` and `.h` from BK's implementation
   - Adapt for Simp project structure

4. **Include in Source**
   ```cpp
   #include <q/pitch/pitch_detector.hpp>
   #include <q/fx/signal_conditioner.hpp>
   #include <q/support/pitch_names.hpp>
   ```

## BK Shepherd Reference

See BK Shepherd's tuner implementation for working Daisy integration example.

**Key Files:**
- `frequency_detector_q.h` - Interface
- `frequency_detector_q.cpp` - Implementation
- `tuner_module.cpp` - Usage example

## API Pattern

```cpp
// Initialize
cycfi::q::pitch_detector detector{lowest_freq, highest_freq, sample_rate};
cycfi::q::signal_conditioner preproc{config, lowest_freq, highest_freq, sample_rate};

// Process
float preprocessed = preproc(raw_input);
bool ready = detector(preprocessed);
if (ready) {
    float frequency = detector.get_frequency();
    // Use frequency...
}
```

## Frequency Range for Guitar

```cpp
frequency lowest_frequency = cycfi::q::pitch_names::C[1];  // ~32 Hz
frequency highest_frequency = cycfi::q::pitch_names::C[5]; // ~523 Hz
```

This covers Low E (82Hz) to High E (330Hz) with headroom.

---

**Status:** Not yet integrated - awaiting project approval
