#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"
#include <stdlib.h>  // For rand() and srand()
#include <cmath>     // For logf()

// Dust is included in daisysp.h - no separate include needed

//
// Ambien Flux v1.0 - Sample & Hold Slicer Delay
// 
// Transform your guitar into a glitch machine. Ambien Flux captures discrete 
// audio slices and plays them back with controllable repetition, randomization,
// and lo-fi processing. Freeze moments in time and manipulate them rhythmically.
//
// ============================================================================
// CONTROLS
// ============================================================================
//
// KNOBS (Always Active):
// - K1: MASTER LEVEL (0-200% / 0dB to +6dB)
// - K2: DRY/WET MIX (0-100%)
//
// TOGGLE 3 UP - Normal Mode:
// - K3: FEEDBACK (0-100% - pattern regeneration)
// - K4: SLICE COUNT (1-16 slices per cycle)
// - K5: SLICE LENGTH (100-500ms with logarithmic curve)
// - K6: STUTTER (0-100% - random repetition/glitch probability)
//
// TOGGLE 3 DOWN - Lo-Fi Mode:
// - K3: WOBBLE (0-100% - tape wow/flutter/uni-vibe character)
// - K4: DUST (0-100% - vinyl crackle density & mix)
// - K5: BIT CRUSH (0-100% - sample rate reduction)
// - K6: (Unused - reserved for future expansion)
//
// TOGGLE 1 - Capture/Playback Modes:
// - UP: Forward capture → Forward playback
// - MIDDLE: Backward capture → Reverse playback
// - DOWN: Forward capture → Random playback direction per slice
//
// TOGGLE 2 - (Unused - reserved for future expansion)
// TOGGLE 3 MIDDLE - (Unused - reserved for future expansion)
//
// FOOTSWITCHES:
// - FS1 TAP: Toggle bypass on/off
// - FS1 HOLD (2 seconds): Enter bootloader for firmware updates
// - FS2 TAP: Toggle freeze (stops capture, loops current buffer)
//
// LEDS:
// - LED1: Effect active (on when not bypassed)
// - LED2: Freeze active (on when frozen)
//
// ============================================================================
// FEATURES
// ============================================================================
//
// CORE SLICING:
// - Sample & hold architecture (discrete buffer capture)
// - 1-16 variable slice count
// - 100-500ms slice length range
// - Zero-crossing detection for click-free transitions
// - Variable crossfade system (15% proportional, 5ms minimum)
// - Read/write conflict protection
//
// PLAYBACK MODES:
// - Forward sequence with forward playback
// - Backward sequence with reverse playback
// - Random playback direction per slice
//
// STUTTER SYSTEM:
// - Shuffle probability (0-100%)
// - Musical subdivision repeats (1x, 2x, 4x, 8x)
// - Random slice selection based on stutter amount
//
// LO-FI EFFECTS:
// - Bit crushing: Sample rate reduction with aggressive 50% Nyquist low-pass
// - Wobble: LFO-modulated delay (0.5-6Hz) for tape wow/flutter/uni-vibe
// - Dust: Sparse vinyl crackle (0-2% density, 600Hz low-pass, progressive mix)
//
// SIGNAL FLOW:
// INPUT → [Bit Crush] → Capture → Slices → Playback
//    ↓                                        ↓
//  Clean Dry                          Processed Wet
//    ↓                                        ↓
//    └────────→ [Dry/Wet Mix] ←──────────────┘
//                     ↓
//                 [Wobble]
//                     ↓
//                  [Dust]
//                     ↓
//              [Master Level]
//                     ↓
//                  OUTPUT
//
// FREEZE MODE:
// - Stops new audio capture
// - Continues playback of frozen buffer
// - All controls remain active (feedback, mix, stutter, playback modes)
// - Build layers with feedback on frozen content
//
// ============================================================================
// TECHNICAL SPECIFICATIONS
// ============================================================================
//
// Platform: Cleveland Music Co. Hothouse (Daisy Seed)
// Sample Rate: 48 kHz
// Bit Depth: 32-bit float internal processing
// Buffer Memory: 16 slices × 24,000 samples (500ms max)
// CPU: STM32H750 @ 480MHz
// Latency: <5ms (buffer-dependent)
//
// ============================================================================
// CREDITS
// ============================================================================
//
// Design & Development: Chris Brandt
// Platform: Cleveland Music Co. Hothouse
// DSP Library: Electro-Smith DaisySP
// Development Tool: Claude.ai
// License: MIT
//
//

using namespace daisy;
using namespace daisysp;
using namespace clevelandmusicco;

// ============================================================================
// ENVELOPE FOLLOWER CLASS - OPTIONAL FEATURE (CURRENTLY DISABLED)
// ============================================================================
// This envelope follower system is fully implemented but commented out.
// It would add dynamic control of slice length & count based on playing dynamics.
// 
// To enable:
// 1. Uncomment this class
// 2. Uncomment envelope-related sections marked "ENVELOPE SYSTEM" below
// 3. Adds Toggle 2 (envelope direction) and Toggle 3 MIDDLE (envelope parameters)
//
/*
class EnvelopeFollower {
public:
    EnvelopeFollower() : envelope_level_(0.0f), samplerate_(48000.0f), 
                         attack_coeff_(0.0f), release_coeff_(0.0f) {}
    
    void Init(float samplerate, float attack_ms, float release_ms) {
        samplerate_ = samplerate;
        setAttackRelease(attack_ms, release_ms);
    }
    
    void setAttackRelease(float attack_ms, float release_ms) {
        attack_coeff_ = 1.0f - expf(-1.0f / (attack_ms * samplerate_ / 1000.0f));
        release_coeff_ = 1.0f - expf(-1.0f / (release_ms * samplerate_ / 1000.0f));
    }
    
    float Process(float input) {
        float input_level = fabsf(input);
        
        if (input_level > envelope_level_) {
            envelope_level_ += attack_coeff_ * (input_level - envelope_level_);
        } else {
            envelope_level_ += release_coeff_ * (input_level - envelope_level_);
        }
        
        return envelope_level_;
    }
    
    float GetEnvelopeLevel() const { return envelope_level_; }
    void Reset() { envelope_level_ = 0.0f; }

private:
    float envelope_level_;
    float samplerate_;
    float attack_coeff_;
    float release_coeff_;
};
*/

// ============================================================================
// CONSTANTS & CONFIGURATION
// ============================================================================

#define MAX_SLICES 16
#define MAX_SLICE_LENGTH 24000  // 500ms @ 48kHz

const float SAMPLE_RATE = 48000.0f;
const float MIN_SLICE_LENGTH_MS = 100.0f;
const float MAX_SLICE_LENGTH_MS = 500.0f;

// ============================================================================
// HARDWARE
// ============================================================================

Hothouse hw;
Led led1, led2;

// ============================================================================
// SLICE BUFFER SYSTEM
// ============================================================================

// Slice buffer array - stores captured audio slices
float DSY_SDRAM_BSS sliceBuffers[MAX_SLICES][MAX_SLICE_LENGTH];

// Slice metadata
int sliceLengths[MAX_SLICES];

// Capture state
int currentCaptureSlice = 0;
int capturePosition = 0;

// Zero-crossing detection for click-free slicing
bool waitingForZeroCrossing = false;
float previousCaptureSample = 0.0f;
bool hasLeftZero = false;
const int MAX_ZERO_SEARCH = 1000;
int zeroSearchCount = 0;

// Playback state
int currentPlaybackSlice = 0;
int playbackPosition = 0;
bool hasContent = false;
bool playbackReverse = false;

// Stutter state
int repeatCount = 0;
int targetRepeats = 1;

// ============================================================================
// DSP MODULES
// ============================================================================

CrossFade mix;
OnePole lofi_filter;  // Low-pass filter for lo-fi downsampling
OnePole dust_filter;  // Low-pass filter to soften dust crackle
Dust dust;            // Sparse random impulses for vinyl crackle
DelayLine<float, 4800> wobble_delay;  // 100ms max delay for wobble (@ 48kHz)
Oscillator wobble_lfo;  // LFO for tape wow/flutter modulation

// ENVELOPE SYSTEM - Uncomment to enable dynamic slice control
// EnvelopeFollower envelope_follower;

// ============================================================================
// CONTROL STATE
// ============================================================================

bool bypass = true;
bool is_frozen = false;  // FS2 tap - latching freeze

// Control values (0.0 - 1.0 from knobs)
float knob_time;
float knob_mix;
float knob_feedback;
float knob_slice_count;
float knob_slice_length;
float knob_stutter;

// Toggle values
int toggle_mode;  // Toggle 1 - Capture/Playback mode
int prev_toggle3_pos = 0;  // Track Toggle 3 position changes

// ENVELOPE SYSTEM - Uncomment to enable
// int toggle2_mode;  // Toggle 2 - Envelope direction (UP/MIDDLE/DOWN)

// Touch detection (BuzzBox pattern)
float knobValues[6] = {0.0f};
float knob_prev[6];
bool knob_touched[6];
const float KNOB_TOUCH_THRESHOLD = 0.02f;

// Lo-Fi Mode control variables
float master_level;
float lofi_wobble;
float lofi_noise;
float lofi_bitcrush;
float lofi_age_mix;

// ENVELOPE SYSTEM - Uncomment to enable
// float env_amount;    // K3 in envelope mode
// float env_attack;    // K4 in envelope mode
// float env_release;   // K5 in envelope mode
// float envelope_value = 0.0f;

// Processed parameters
int active_slice_count;
float slice_length_ms;
int slice_length_samples;
float slice_length_samples_smooth;
float feedback_amount;

// ============================================================================
// LO-FI BIT CRUSHING
// ============================================================================

// Static variables for sample & hold state
static float bitcrush_hold_sample = 0.0f;
static int bitcrush_sample_counter = 0;

float CustomBitCrush(float input, float amount)
{
    // If amount is 0, bypass processing completely
    if (amount <= 0.0f) return input;
    
    // Map amount to downsample rate
    // 0.0 = no downsample (bypassed above)
    // 1.0 = heavy downsample (32 sample hold)
    int downsample_rate = 1 + (int)(amount * amount * 31.0f);  // 1 to 32 samples
    
    // Set filter cutoff based on effective sample rate - MORE AGGRESSIVE
    // Nyquist frequency = (48000 / downsample_rate) / 2
    float effective_nyquist = (48000.0f / (float)downsample_rate) / 2.0f;
    float cutoff = effective_nyquist * 0.5f;  // 50% of Nyquist for aggressive lo-fi character
    if (cutoff > 18000.0f) cutoff = 18000.0f;
    if (cutoff < 500.0f) cutoff = 500.0f;
    lofi_filter.SetFrequency(cutoff);
    
    // Only update held sample at downsample rate
    if (bitcrush_sample_counter >= downsample_rate) {
        bitcrush_sample_counter = 0;
        bitcrush_hold_sample = input;
    }
    
    bitcrush_sample_counter++;
    
    // Apply low-pass filter to remove aliasing artifacts
    return lofi_filter.Process(bitcrush_hold_sample);
}

// ============================================================================
// CONTROL PROCESSING
// ============================================================================

void UpdateControls()
{
    hw.ProcessAllControls();
    
    // Read ALL knobs into array (BuzzBox pattern)
    knobValues[0] = hw.GetKnobValue(Hothouse::KNOB_1);
    knobValues[1] = hw.GetKnobValue(Hothouse::KNOB_2);
    knobValues[2] = hw.GetKnobValue(Hothouse::KNOB_3);
    knobValues[3] = hw.GetKnobValue(Hothouse::KNOB_4);
    knobValues[4] = hw.GetKnobValue(Hothouse::KNOB_5);
    knobValues[5] = hw.GetKnobValue(Hothouse::KNOB_6);
    
    // Detect knob movement for K3-K6 (BuzzBox pattern)
    for(int i = 2; i < 6; i++) {
        if (fabsf(knobValues[i] - knob_prev[i]) > KNOB_TOUCH_THRESHOLD) {
            knob_touched[i] = true;
            knob_prev[i] = knobValues[i];
        }
    }
    
    // Check Toggle 3 position (0=Normal, 1=Envelope, 2=Lo-Fi)
    int toggle3_pos = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3);
    
    // Detect Toggle 3 position change - reset touch flags (BuzzBox pattern)
    if (toggle3_pos != prev_toggle3_pos) {
        prev_toggle3_pos = toggle3_pos;
        
        // Reset touch flags for K3-K6 so parameters don't jump on mode switch
        knob_touched[2] = false;
        knob_touched[3] = false;
        knob_touched[4] = false;
        knob_touched[5] = false;
        
        // Capture current knob positions so we can detect movement from here
        knob_prev[2] = knobValues[2];
        knob_prev[3] = knobValues[3];
        knob_prev[4] = knobValues[4];
        knob_prev[5] = knobValues[5];
    }
    
    int shift_mode = toggle3_pos;  // 0=Normal, 1=Envelope, 2=Lo-Fi
    
    // Read Toggle 1 for capture/playback mode
    toggle_mode = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
    
    // ENVELOPE SYSTEM - Uncomment to enable
    // toggle2_mode = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2);
    
    // Map controls based on shift mode
    float k1 = knobValues[0];
    float k2 = knobValues[1];
    float k3 = knobValues[2];
    float k4 = knobValues[3];
    float k5 = knobValues[4];
    float k6 = knobValues[5];
    
    // K1 and K2 are GLOBAL - always active
    master_level = k1 * 2.0f;  // 0.0 - 2.0 (0dB to +6dB)
    knob_mix = k2;
    
    // Toggle 3: UP = Normal Mode, DOWN = Lo-Fi Mode
    if (shift_mode == 0) {
        // NORMAL MODE - Core slicing parameters
        if (knob_touched[2]) knob_feedback = k3;
        if (knob_touched[3]) knob_slice_count = k4;
        if (knob_touched[4]) knob_slice_length = k5;
        if (knob_touched[5]) knob_stutter = k6;
        
    } else {
        // LO-FI MODE - Degradation effects
        if (knob_touched[2]) lofi_wobble = k3;
        if (knob_touched[3]) lofi_noise = k4;
        if (knob_touched[4]) lofi_bitcrush = k5;
    }
    
    // ENVELOPE SYSTEM - Uncomment to enable Toggle 3 MIDDLE mode
    /*
    else if (shift_mode == 1) {
        // ENVELOPE MODE - Dynamic control parameters
        if (knob_touched[2]) env_amount = k3;
        if (knob_touched[3]) env_attack = k4;
        if (knob_touched[4]) env_release = k5;
    }
    */
}

void UpdateButtons()
{
    // FS1 - Bypass toggle
    if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }
    
    // FS2 - Freeze toggle (latching)
    if (hw.switches[Hothouse::FOOTSWITCH_2].RisingEdge()) {
        is_frozen = !is_frozen;
    }
}

void UpdateLEDs()
{
    // LED1 - Effect active
    led1.Set(bypass ? 0.0f : 1.0f);
    
    // LED2 - Freeze indicator
    led2.Set(is_frozen ? 1.0f : 0.0f);
    
    led1.Update();
    led2.Update();
}

void ProcessParameters()
{
    float base_slice_count = knob_slice_count;
    float base_slice_length = knob_slice_length;
    
    // ENVELOPE SYSTEM - Uncomment to enable dynamic modulation
    /*
    if (toggle2_mode != 2 && env_amount > 0.01f) {
        float env_mod = envelope_value;
        if (toggle2_mode == 1) env_mod = 1.0f - env_mod;
        
        float count_mod = 1.0f + ((env_mod - 0.5f) * env_amount);
        base_slice_count = base_slice_count * count_mod;
        if (base_slice_count < 0.0f) base_slice_count = 0.0f;
        if (base_slice_count > 1.0f) base_slice_count = 1.0f;
        
        float length_mod = 1.0f + ((env_mod - 0.5f) * env_amount);
        base_slice_length = base_slice_length * length_mod;
        if (base_slice_length < 0.0f) base_slice_length = 0.0f;
        if (base_slice_length > 1.0f) base_slice_length = 1.0f;
    }
    */
    
    // Map K4 to slice count (1-16)
    active_slice_count = (int)(base_slice_count * 15.999f) + 1;
    
    // Map K4 to slice count (1-16)
    active_slice_count = (int)(base_slice_count * 15.999f) + 1;
    if (active_slice_count < 1) active_slice_count = 1;
    if (active_slice_count > MAX_SLICES) active_slice_count = MAX_SLICES;
    
    // Map K5 to slice length (100-500ms) with logarithmic curve
    float log_knob = logf(1.0f + 9.0f * base_slice_length) / logf(10.0f);
    slice_length_ms = MIN_SLICE_LENGTH_MS + 
                      (log_knob * (MAX_SLICE_LENGTH_MS - MIN_SLICE_LENGTH_MS));
    
    slice_length_samples = (int)((slice_length_ms / 1000.0f) * SAMPLE_RATE);
    
    if (slice_length_samples < 1) slice_length_samples = 1;
    if (slice_length_samples > MAX_SLICE_LENGTH) slice_length_samples = MAX_SLICE_LENGTH;
    
    // Map K3 to feedback
    feedback_amount = knob_feedback;
}

// ============================================================================
// K6 STUTTER SYSTEM
// ============================================================================

int CalculateRepeatCount(float stutter_knob)
{
    if (stutter_knob < 0.01f) {
        return 1;
    }
    
    float shuffle_probability = stutter_knob;
    
    if ((rand() % 100) < (int)(shuffle_probability * 100.0f)) {
        int subdivision_choice = rand() % 100;
        
        if (subdivision_choice < 40) {
            return 2;
        } else if (subdivision_choice < 70) {
            return 4;
        } else if (subdivision_choice < 90) {
            return 1;
        } else {
            return 8;
        }
    }
    
    return 1;
}

int GetNextSlice(int currentSlice, int sliceCount, float stutterKnob, int toggleMode, bool& reverseFlag)
{
    int nextSlice;
    
    if (toggleMode == 2) {
        nextSlice = rand() % sliceCount;
        reverseFlag = (rand() % 2) == 0;
    } else if (toggleMode == 1) {
        nextSlice = currentSlice - 1;
        if (nextSlice < 0) {
            nextSlice = sliceCount - 1;
        }
        reverseFlag = true;
    } else {
        nextSlice = currentSlice + 1;
        if (nextSlice >= sliceCount) {
            nextSlice = 0;
        }
        reverseFlag = false;
    }
    
    return nextSlice;
}

// ============================================================================
// SLICE CAPTURE ENGINE
// ============================================================================

void InitializeSliceBuffers()
{
    for (int slice = 0; slice < MAX_SLICES; slice++) {
        sliceLengths[slice] = 0;
        for (int sample = 0; sample < MAX_SLICE_LENGTH; sample++) {
            sliceBuffers[slice][sample] = 0.0f;
        }
    }
    
    currentCaptureSlice = 0;
    currentPlaybackSlice = 0;
    capturePosition = 0;
    playbackPosition = 0;
    hasContent = false;
    playbackReverse = false;
    
    waitingForZeroCrossing = false;
    previousCaptureSample = 0.0f;
    hasLeftZero = false;
    zeroSearchCount = 0;
}

void CaptureSlice(float input)
{
    // Skip capture if frozen - keeps current buffer contents
    if (is_frozen) {
        return;
    }
    
    if (fabsf(input) > 0.01f) {
        hasLeftZero = true;
    }
    
    bool zeroCrossing = false;
    if (hasLeftZero && 
        ((previousCaptureSample > 0.0f && input <= 0.0f) ||
         (previousCaptureSample < 0.0f && input >= 0.0f))) {
        zeroCrossing = true;
    }
    
    sliceBuffers[currentCaptureSlice][capturePosition] = input;
    capturePosition++;
    
    previousCaptureSample = input;
    
    bool shouldFinalize = false;
    
    if (waitingForZeroCrossing) {
        zeroSearchCount++;
        
        if (zeroCrossing) {
            shouldFinalize = true;
        } else if (zeroSearchCount >= MAX_ZERO_SEARCH) {
            shouldFinalize = true;
        } else if (capturePosition >= MAX_SLICE_LENGTH - 1) {
            shouldFinalize = true;
        }
    } else if (capturePosition >= (int)slice_length_samples_smooth) {
        waitingForZeroCrossing = true;
        zeroSearchCount = 0;
        hasLeftZero = false;
    }
    
    if (shouldFinalize) {
        sliceLengths[currentCaptureSlice] = capturePosition;
        
        if (!hasContent) {
            hasContent = true;
            currentPlaybackSlice = currentCaptureSlice;
            playbackPosition = 0;
            repeatCount = 0;
            targetRepeats = CalculateRepeatCount(knob_stutter);
            
            if (toggle_mode == 2) {
                playbackReverse = (rand() % 2) == 0;
            } else if (toggle_mode == 1) {
                playbackReverse = true;
            } else {
                playbackReverse = false;
            }
        }
        
        if (toggle_mode == 2) {
            currentCaptureSlice = rand() % active_slice_count;
        } else {
            currentCaptureSlice++;
            if (currentCaptureSlice >= active_slice_count) {
                currentCaptureSlice = 0;
            }
        }
        
        capturePosition = 0;
        waitingForZeroCrossing = false;
        zeroSearchCount = 0;
        hasLeftZero = false;
        previousCaptureSample = 0.0f;
    }
}

// ============================================================================
// SLICE PLAYBACK ENGINE
// ============================================================================

float PlaybackSlice()
{
    if (!hasContent) {
        return 0.0f;
    }
    
    if (currentPlaybackSlice == currentCaptureSlice) {
        currentPlaybackSlice = GetNextSlice(currentPlaybackSlice, active_slice_count, knob_stutter, toggle_mode, playbackReverse);
        playbackPosition = 0;
        repeatCount = 0;
        targetRepeats = CalculateRepeatCount(knob_stutter);
    }
    
    if (sliceLengths[currentPlaybackSlice] == 0) {
        return 0.0f;
    }
    
    int readPosition;
    if (playbackReverse) {
        int sliceLen = sliceLengths[currentPlaybackSlice];
        if (playbackPosition < sliceLen) {
            readPosition = sliceLen - 1 - playbackPosition;
        } else {
            readPosition = 0;
        }
    } else {
        readPosition = playbackPosition;
    }
    
    if (readPosition < 0 || readPosition >= sliceLengths[currentPlaybackSlice]) {
        readPosition = 0;
    }
    
    float output = sliceBuffers[currentPlaybackSlice][readPosition];
    
    // Variable crossfade
    static int lastPlayedSlice = -1;
    static int fadeLength = 0;
    
    bool sliceChanged = (currentPlaybackSlice != lastPlayedSlice) || (playbackPosition == 0);
    if (sliceChanged) {
        lastPlayedSlice = currentPlaybackSlice;
        int currentSliceLen = sliceLengths[currentPlaybackSlice];
        fadeLength = currentSliceLen * 15 / 100;
        if (fadeLength < 240) fadeLength = 240;
        
        if (fadeLength * 2 > currentSliceLen) {
            fadeLength = currentSliceLen / 3;
            if (fadeLength < 1) fadeLength = 1;
        }
    }
    
    float fadeEnvelope = 1.0f;
    int currentSliceLen = sliceLengths[currentPlaybackSlice];
    
    if (playbackPosition < fadeLength) {
        fadeEnvelope = (float)playbackPosition / (float)fadeLength;
    }
    
    int fadeOutStart = currentSliceLen - fadeLength;
    if (fadeOutStart > 0 && playbackPosition >= fadeOutStart) {
        int fadeOutPos = playbackPosition - fadeOutStart;
        float fadeOutEnvelope = 1.0f - ((float)fadeOutPos / (float)fadeLength);
        if (fadeOutEnvelope < fadeEnvelope) {
            fadeEnvelope = fadeOutEnvelope;
        }
    }
    
    output *= fadeEnvelope;
    
    playbackPosition++;
    
    if (playbackPosition >= sliceLengths[currentPlaybackSlice]) {
        repeatCount++;
        playbackPosition = 0;
        
        if (repeatCount >= targetRepeats) {
            int nextSlice = GetNextSlice(currentPlaybackSlice, active_slice_count, knob_stutter, toggle_mode, playbackReverse);
            
            if (nextSlice == currentCaptureSlice) {
                nextSlice = GetNextSlice(nextSlice, active_slice_count, knob_stutter, toggle_mode, playbackReverse);
            }
            
            if (sliceLengths[nextSlice] > 0) {
                currentPlaybackSlice = nextSlice;
                playbackPosition = 0;
            }
            
            repeatCount = 0;
            targetRepeats = CalculateRepeatCount(knob_stutter);
        }
    }
    
    return output;
}

// ============================================================================
// AUDIO CALLBACK
// ============================================================================

static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out,
                          size_t size)
{
    UpdateControls();
    UpdateButtons();
    UpdateLEDs();
    ProcessParameters();
    
    for (size_t i = 0; i < size; i++)
    {
        fonepole(slice_length_samples_smooth, (float)slice_length_samples, 0.0002f);
        
        float input = in[0][i];
        float dry_input = input;
        
        // ENVELOPE SYSTEM - Uncomment to enable
        /*
        if (toggle2_mode != 2) {
            float attack_time = 1.0f + (env_attack * 199.0f);
            float release_time = 10.0f + (env_release * 990.0f);
            envelope_follower.setAttackRelease(attack_time, release_time);
            envelope_value = envelope_follower.Process(input);
        } else {
            envelope_value = 0.5f;
        }
        */
        
        float output;
        
        if (!bypass) {
            // Apply lo-fi bit crushing to input BEFORE capture
            // This affects what gets captured into slices (vintage sampler aesthetic)
            // When lofi_bitcrush = 0, CustomBitCrush returns input unchanged
            float processed_input = CustomBitCrush(input, lofi_bitcrush);
            
            // Read from playback engine
            float wet = PlaybackSlice();
            
            // Apply feedback using processed input
            float capture_input = processed_input + (wet * feedback_amount);
            
            // Capture with feedback applied
            CaptureSlice(capture_input);
            
            // Dry/wet mix using CLEAN dry signal and processed wet
            mix.SetPos(knob_mix);
            output = mix.Process(dry_input, wet);
            
            // Apply wobble AFTER mix (tape wow/flutter / uni-vibe character)
            // Wobble runs whenever lofi_wobble > 0 AND mix > 0
            // Creates pitch modulation via LFO-modulated delay
            if (lofi_wobble > 0.0f && knob_mix > 0.01f) {
                // LFO rate: 0.5Hz to 6Hz with curve for musical control
                // Low settings = slow tape drift, high settings = vibrato/uni-vibe
                float lfo_rate = 0.5f + (lofi_wobble * lofi_wobble * 5.5f);  // 0.5Hz to 6Hz
                wobble_lfo.SetFreq(lfo_rate);
                
                // Get LFO value (-1.0 to 1.0)
                float lfo_value = wobble_lfo.Process();
                
                // Map to delay time: 2ms to 8ms range for noticeable pitch movement
                // Progressive depth - more wobble at higher settings
                float delay_depth_ms = 2.0f + (lofi_wobble * 6.0f);  // 2ms to 8ms
                float center_delay_ms = 5.0f;  // Center point
                float delay_time_ms = center_delay_ms + (lfo_value * delay_depth_ms * 0.5f);
                
                // Convert to samples
                float delay_samples = (delay_time_ms / 1000.0f) * SAMPLE_RATE;
                wobble_delay.SetDelay(delay_samples);
                
                // Write current output to delay
                wobble_delay.Write(output);
                
                // Read modulated signal
                float wobbled = wobble_delay.Read();
                
                // Reduced maximum mix for more usable knob range
                // Max 50% wet instead of 100% - keeps more of the original character
                float wobble_mix = lofi_wobble * lofi_wobble * 0.5f;  // Max 50% at full knob
                output = output * (1.0f - wobble_mix) + wobbled * wobble_mix;
            }
            
            // Apply dust AFTER wobble, BEFORE master level (vinyl-on-top aesthetic)
            // Dust runs whenever lofi_noise > 0 AND mix > 0, regardless of toggle position
            // This way dust only appears on the wet signal - dry signal stays completely clean
            if (lofi_noise > 0.0f && knob_mix > 0.01f) {
                // Very conservative density: 0-2% range with progressive curve
                float density = lofi_noise * lofi_noise * 0.02f;  // Squared curve, max 2%
                dust.SetDensity(density);
                
                float dust_signal = dust.Process();  // 0.0 to 1.0, sparse impulses
                dust_signal = dust_filter.Process(dust_signal);  // 600Hz filter for warmth
                
                // Progressive mix: starts very subtle, ramps up
                float mix_amount = lofi_noise * lofi_noise * 0.05f;  // Max 5% mix
                output += (dust_signal - 0.5f) * mix_amount;
            }
            
            // Apply master level (controls everything: mix + dust)
            output *= master_level;
            
        } else {
            output = input;
        }
        
        out[0][i] = output;
        out[1][i] = output;
    }
}

// ============================================================================
// MAIN INITIALIZATION
// ============================================================================

int main(void)
{
    hw.Init(true);
    
    srand(System::GetNow());
    
    hw.SetAudioBlockSize(512);
    
    InitializeSliceBuffers();
    
    mix.Init();
    
    // Initialize lo-fi filter
    lofi_filter.Init();
    lofi_filter.SetFrequency(8000.0f);
    
    // Initialize dust filter (very low frequency for warm vinyl character)
    dust_filter.Init();
    dust_filter.SetFrequency(600.0f);  // Warm vinyl character
    
    // Initialize dust for lo-fi crackle
    dust.Init();
    
    // Initialize wobble effect
    wobble_delay.Init();
    wobble_lfo.Init(SAMPLE_RATE);
    wobble_lfo.SetWaveform(Oscillator::WAVE_SIN);  // Smooth sine wave for natural tape flutter
    wobble_lfo.SetFreq(1.0f);  // Default 1Hz
    wobble_lfo.SetAmp(1.0f);
    
    // ENVELOPE SYSTEM - Uncomment to enable
    // envelope_follower.Init(SAMPLE_RATE, 50.0f, 100.0f);
    
    bypass = true;
    is_frozen = false;
    
    knob_time = 0.0f;
    knob_mix = 0.5f;
    knob_feedback = 0.3f;
    knob_slice_count = 0.25f;
    knob_slice_length = 0.4f;
    knob_stutter = 0.0f;
    toggle_mode = 0;
    
    // ENVELOPE SYSTEM - Uncomment to enable
    // toggle2_mode = 2;
    
    // Initialize touch detection
    for (int i = 0; i < 6; i++) {
        knob_prev[i] = 0.0f;
        knob_touched[i] = true;
    }
    
    master_level = 1.0f;
    lofi_wobble = 0.0f;
    lofi_noise = 0.0f;
    lofi_bitcrush = 0.0f;
    lofi_age_mix = 0.0f;
    
    ProcessParameters();
    slice_length_samples_smooth = (float)slice_length_samples;
    
    repeatCount = 0;
    targetRepeats = 1;
    
    led1.Init(hw.seed.GetPin(Hothouse::LED_1), false);
    led2.Init(hw.seed.GetPin(Hothouse::LED_2), false);
    led1.Set(0.0f);
    led2.Set(0.0f);
    led1.Update();
    led2.Update();
    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    while(1)
    {
        if(hw.switches[Hothouse::FOOTSWITCH_1].TimeHeldMs() >= 2000)
        {
            hw.StopAudio();
            hw.StopAdc();
            
            for(int i = 0; i < 3; i++) 
            {
                led1.Set(1.0f);
                led2.Set(0.0f);
                led1.Update();
                led2.Update();
                System::Delay(100);
                
                led1.Set(0.0f);
                led2.Set(1.0f);
                led1.Update();
                led2.Update();
                System::Delay(100);
            }
            
            System::ResetToBootloader();
        }
        
        System::Delay(100);
    }
}
