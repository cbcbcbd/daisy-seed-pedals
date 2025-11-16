#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"
#include <stdlib.h>  // For rand() and srand()
#include <cmath>     // For logf()

//
// Ambien Delay v1.0 - Ambient Granular Delay
//
// Platform: Cleveland Music Co. Hothouse
// Development Stage: GUIDED
// Architecture: Sample & Hold (discrete buffer capture)
//

using namespace daisy;
using namespace daisysp;
using namespace clevelandmusicco;

// ============================================================================
// CONSTANTS & CONFIGURATION
// ============================================================================

#define MAX_SLICES 16
#define MAX_SLICE_LENGTH 24000  // 500ms @ 48kHz

const float SAMPLE_RATE = 48000.0f;
const float MIN_SLICE_LENGTH_MS = 100.0f;  // Increased from 50ms to ensure musical content after crossfades
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
int sliceLengths[MAX_SLICES];           // Actual length of each slice in samples

// Capture state
int currentCaptureSlice = 0;            // Which slice we're currently capturing into
int capturePosition = 0;                // Current write position within slice

// Zero-crossing detection for click-free slicing
bool waitingForZeroCrossing = false;    // Flag when slice is full, searching for zero
float previousCaptureSample = 0.0f;     // Last sample for zero-crossing detection
bool hasLeftZero = false;               // Hysteresis - signal must leave zero before detecting crossing
const int MAX_ZERO_SEARCH = 1000;       // Max samples to search (~20ms safety limit)
int zeroSearchCount = 0;                // How long we've been searching

// Playback state
int currentPlaybackSlice = 0;           // Which slice we're currently playing
int playbackPosition = 0;               // Current read position within slice
bool hasContent = false;                // Have we captured anything yet?
bool playbackReverse = false;           // Current slice playing in reverse? (for mode 2)

// Stutter state
int repeatCount = 0;                    // How many times we've repeated current slice
int targetRepeats = 1;                  // How many times to repeat current slice

// ============================================================================
// DSP MODULES
// ============================================================================

CrossFade mix;  // For dry/wet blending

// ============================================================================
// CONTROL STATE
// ============================================================================

bool bypass = true;

// Control values (0.0 - 1.0 from knobs)
float knob_time;         // K1 (reserved for pre-delay in future phases)
float knob_mix;          // K2 - Dry/wet blend
float knob_feedback;     // K3 - Feedback amount
float knob_slice_count;  // K4 - Number of slices (1-16)
float knob_slice_length; // K5 - Length of each slice (10-500ms)
float knob_stutter;      // K6 - Stutter/shuffle control

// Toggle values (0, 1, or 2 for UP/MIDDLE/DOWN)
int toggle_mode;         // Toggle 1 - Capture/Playback mode

// Processed parameters
int active_slice_count;         // 1-16
float slice_length_ms;          // 10-500ms
int slice_length_samples;       // Converted to samples
float slice_length_samples_smooth; // Smoothed version (to prevent clicks) - MUST be float for fonepole
float feedback_amount;          // 0.0-1.0

// ============================================================================
// CONTROL PROCESSING
// ============================================================================

void UpdateControls()
{
    hw.ProcessAllControls();
    
    // Read knobs
    knob_time = hw.GetKnobValue(Hothouse::KNOB_1);          // Reserved for pre-delay
    knob_mix = hw.GetKnobValue(Hothouse::KNOB_2);           // Dry/wet mix
    knob_feedback = hw.GetKnobValue(Hothouse::KNOB_3);      // Feedback
    knob_slice_count = hw.GetKnobValue(Hothouse::KNOB_4);   // Slice count
    knob_slice_length = hw.GetKnobValue(Hothouse::KNOB_5);  // Slice length
    knob_stutter = hw.GetKnobValue(Hothouse::KNOB_6);       // Stutter/shuffle
    
    // Read toggles
    toggle_mode = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);  // 0=Forward, 1=Reverse, 2=Random
}

void UpdateButtons()
{
    // FS1 - Bypass toggle
    if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }
    
    // FS2 - Reserved for FREEZE mode (Phase 6)
}

void UpdateLEDs()
{
    // LED1 - Effect active (on when not bypassed)
    led1.Set(bypass ? 0.0f : 1.0f);
    
    // LED2 - Reserved for freeze mode (Phase 6)
    led2.Set(0.0f);
    
    led1.Update();
    led2.Update();
}

void ProcessParameters()
{
    // Map K4 (0.0-1.0) to slice count (1-16)
    // Use rounding instead of truncation to eliminate dead zone
    active_slice_count = (int)(knob_slice_count * 15.999f) + 1;
    if (active_slice_count < 1) active_slice_count = 1;
    if (active_slice_count > MAX_SLICES) active_slice_count = MAX_SLICES;
    
    // Map K5 (0.0-1.0) to slice length (100-500ms) with logarithmic curve
    // Proper log curve gives better resolution in the musical range
    // log(1 + 9*x) / log(10) creates smooth logarithmic response
    float log_knob = logf(1.0f + 9.0f * knob_slice_length) / logf(10.0f);
    slice_length_ms = MIN_SLICE_LENGTH_MS + 
                      (log_knob * (MAX_SLICE_LENGTH_MS - MIN_SLICE_LENGTH_MS));
    
    // Convert to samples
    slice_length_samples = (int)((slice_length_ms / 1000.0f) * SAMPLE_RATE);
    
    // Clamp to valid range
    if (slice_length_samples < 1) slice_length_samples = 1;
    if (slice_length_samples > MAX_SLICE_LENGTH) slice_length_samples = MAX_SLICE_LENGTH;
    
    // Map K3 (0.0-1.0) to feedback (0-100%, limited for Phase 1)
    feedback_amount = knob_feedback;
}

// ============================================================================
// K6 STUTTER SYSTEM
// ============================================================================

// Calculate repeat count based on K6 - favors musical subdivisions
int CalculateRepeatCount(float k6_value) {
    if (k6_value < 0.01f) return 1;  // K6 at 0 = no repeats
    
    float random_val = (float)(rand() % 10000) / 10000.0f;
    
    // Weight distribution based on K6 - favors musical subdivisions (1x, 2x, 4x, 8x)
    if (k6_value < 0.25f) {
        // Low: mostly 1x, occasional 2x
        return (random_val < 0.95f) ? 1 : 2;
    } else if (k6_value < 0.50f) {
        // Medium-low: mostly 2x, some 1x and 4x
        if (random_val < 0.15f) return 1;
        if (random_val < 0.70f) return 2;
        return 4;
    } else if (k6_value < 0.75f) {
        // Medium-high: mostly 4x, some 2x and 8x
        if (random_val < 0.15f) return 2;
        if (random_val < 0.70f) return 4;
        return 8;
    } else {
        // High: mostly 8x, some 4x
        return (random_val < 0.25f) ? 4 : 8;
    }
}

// Calculate shuffle probability based on K6
// Returns true if we should jump to a random slice instead of sequential
bool ShouldShuffle(float k6_value) {
    if (k6_value < 0.01f) return false;  // K6 at 0 = no shuffle
    
    // Shuffle probability increases with K6
    // 0% at K6=0, ramping up to ~95% at K6=100%
    // Lower threshold (0.05) makes shuffle start earlier
    float shuffle_probability = k6_value * 0.95f;
    
    float random_val = (float)(rand() % 10000) / 10000.0f;
    return (random_val < shuffle_probability);
}

// ============================================================================
// TOGGLE 1 MODE SYSTEM
// ============================================================================

// Get next slice index based on current mode and shuffle state
// Mode 0 (UP): Forward slice sequence, forward playback direction
// Mode 1 (MIDDLE): Backward slice sequence, reverse playback direction
// Mode 2 (DOWN): Forward slice sequence, RANDOM playback direction per slice
int GetNextSlice(int currentSlice, int maxSlices, float k6_value, int mode, bool& reverse_flag) {
    // Determine if we should shuffle (jump randomly) or go sequential
    bool shuffle = ShouldShuffle(k6_value);
    
    if (shuffle) {
        // Jump to random slice (0 to maxSlices-1)
        int randomSlice = rand() % maxSlices;
        
        // Mode 2: Randomly decide playback direction for this slice
        if (mode == 2) {
            reverse_flag = (rand() % 2 == 0);  // 50/50 chance
        }
        
        return randomSlice;
    } else {
        // Sequential advancement based on mode
        int nextSlice;
        
        if (mode == 1) {
            // Mode 1 (MIDDLE): Backward sequence
            nextSlice = currentSlice - 1;
            if (nextSlice < 0) nextSlice = maxSlices - 1;  // Wrap to end
            reverse_flag = true;  // Always reverse in mode 1
        } else if (mode == 2) {
            // Mode 2 (DOWN): Forward sequence, random direction per slice
            nextSlice = currentSlice + 1;
            if (nextSlice >= maxSlices) nextSlice = 0;  // Wrap to start
            reverse_flag = (rand() % 2 == 0);  // Random direction for next slice
        } else {
            // Mode 0 (UP): Forward sequence, forward playback
            nextSlice = currentSlice + 1;
            if (nextSlice >= maxSlices) nextSlice = 0;  // Wrap to start
            reverse_flag = false;  // Always forward in mode 0
        }
        
        return nextSlice;
    }
}

// ============================================================================
// SLICE BUFFER INITIALIZATION
// ============================================================================

void InitializeSliceBuffers()
{
    // Clear all slice buffers and metadata
    for (int slice = 0; slice < MAX_SLICES; slice++) {
        sliceLengths[slice] = 0;  // Mark all slices as empty
        for (int sample = 0; sample < MAX_SLICE_LENGTH; sample++) {
            sliceBuffers[slice][sample] = 0.0f;
        }
    }
    
    // Initialize capture state
    currentCaptureSlice = 0;
    capturePosition = 0;
    
    // Initialize playback state
    currentPlaybackSlice = 0;
    playbackPosition = 0;
    hasContent = false;
    
    // Initialize zero-crossing detection
    waitingForZeroCrossing = false;
    previousCaptureSample = 0.0f;
    hasLeftZero = false;
    zeroSearchCount = 0;
}

// ============================================================================
// SLICE CAPTURE WITH ZERO-CROSSING DETECTION
// ============================================================================

void CaptureSlice(float input)
{
    // Convert smoothed float to int for array access
    int target_length = (int)slice_length_samples_smooth;
    
    // Safety check for array bounds
    if (target_length > MAX_SLICE_LENGTH) {
        target_length = MAX_SLICE_LENGTH;
    }
    
    // If we're waiting for a zero-crossing, keep searching
    if (waitingForZeroCrossing) {
        zeroSearchCount++;
        
        // Check for zero-crossing with hysteresis
        // Signal must leave zero region before we detect a crossing
        const float ZERO_THRESHOLD = 0.01f;  // 1% threshold
        
        if (!hasLeftZero) {
            // Wait for signal to leave zero region
            if (fabsf(input) > ZERO_THRESHOLD) {
                hasLeftZero = true;
                previousCaptureSample = input;
            }
        } else {
            // Signal has left zero - now look for crossing back through zero
            bool crossingDetected = (previousCaptureSample > 0.0f && input <= 0.0f) || 
                                   (previousCaptureSample < 0.0f && input >= 0.0f);
            
            if (crossingDetected || zeroSearchCount >= MAX_ZERO_SEARCH) {
                // Found zero-crossing OR safety timeout reached
                // Finalize current slice and advance to next
                sliceLengths[currentCaptureSlice] = capturePosition;
                
                currentCaptureSlice++;
                if (currentCaptureSlice >= active_slice_count) {
                    currentCaptureSlice = 0;  // Wrap to first slice
                }
                
                capturePosition = 0;
                waitingForZeroCrossing = false;
                hasLeftZero = false;
                zeroSearchCount = 0;
                hasContent = true;  // We've captured at least one complete slice
            }
        }
        
        previousCaptureSample = input;
        
        // Continue capturing during zero-crossing search
        // This prevents gaps in the captured audio
        if (capturePosition < MAX_SLICE_LENGTH) {
            sliceBuffers[currentCaptureSlice][capturePosition] = input;
            capturePosition++;
        }
        
        return;
    }
    
    // Normal capture - write input to current position
    sliceBuffers[currentCaptureSlice][capturePosition] = input;
    capturePosition++;
    
    // Check if we've filled this slice to target length
    if (capturePosition >= target_length) {
        // Start searching for zero-crossing
        waitingForZeroCrossing = true;
        hasLeftZero = false;
        zeroSearchCount = 0;
        previousCaptureSample = input;
    }
}

// ============================================================================
// SLICE PLAYBACK WITH VARIABLE CROSSFADE
// ============================================================================

float PlaybackSlice()
{
    // Don't play anything until we have content
    if (!hasContent || sliceLengths[currentPlaybackSlice] <= 0) {
        return 0.0f;
    }
    
    // Calculate read position - REVERSE MODE for mode 1 and mode 2 random direction
    int readPosition;
    if (playbackReverse) {
        // Reverse playback - start from end of slice, read backwards
        readPosition = sliceLengths[currentPlaybackSlice] - 1 - playbackPosition;
        if (readPosition < 0) readPosition = 0;  // Safety clamp
    } else {
        // Forward playback
        readPosition = playbackPosition;
    }
    
    // Safety bounds check
    if (readPosition >= sliceLengths[currentPlaybackSlice]) {
        readPosition = sliceLengths[currentPlaybackSlice] - 1;
    }
    if (readPosition < 0) readPosition = 0;
    
    // Read sample from buffer
    float output = sliceBuffers[currentPlaybackSlice][readPosition];
    
    // Apply variable crossfade at slice transitions (proportional to slice length)
    // Fade in at start, fade out at end - MUST work for both forward and reverse playback
    static int lastPlayedSlice = -1;
    static int fadeLength = 0;
    
    // Detect slice change (including repeats - we want fade on EVERY repeat)
    bool sliceChanged = (currentPlaybackSlice != lastPlayedSlice) || (playbackPosition == 0);
    if (sliceChanged) {
        lastPlayedSlice = currentPlaybackSlice;
        // Calculate fade length as 15% of slice length
        // Minimum 240 samples (5ms)
        int currentSliceLen = sliceLengths[currentPlaybackSlice];
        fadeLength = currentSliceLen * 15 / 100;
        if (fadeLength < 240) fadeLength = 240;
        
        // Prevent overlapping fades on very short slices
        // If slice is too short for both fades, reduce fade length
        if (fadeLength * 2 > currentSliceLen) {
            fadeLength = currentSliceLen / 3;  // Use 33% for fade in + out
            if (fadeLength < 1) fadeLength = 1;
        }
    }
    
    float fadeEnvelope = 1.0f;
    int currentSliceLen = sliceLengths[currentPlaybackSlice];
    
    // Fade in at start of slice (based on playbackPosition, works for both forward/reverse)
    if (playbackPosition < fadeLength) {
        fadeEnvelope = (float)playbackPosition / (float)fadeLength;
    }
    
    // Fade out at end of slice
    int fadeOutStart = currentSliceLen - fadeLength;
    if (fadeOutStart > 0 && playbackPosition >= fadeOutStart) {
        int fadeOutPos = playbackPosition - fadeOutStart;
        float fadeOutEnvelope = 1.0f - ((float)fadeOutPos / (float)fadeLength);
        // Use minimum of fade-in and fade-out to prevent overlap
        if (fadeOutEnvelope < fadeEnvelope) {
            fadeEnvelope = fadeOutEnvelope;
        }
    }
    
    output *= fadeEnvelope;
    
    // Advance playback position
    playbackPosition++;
    
    // Check if we've reached the end of this slice
    if (playbackPosition >= sliceLengths[currentPlaybackSlice]) {
        repeatCount++;  // Completed one play of this slice
        playbackPosition = 0;  // Loop back to start of slice
        
        // Check if we've repeated this slice enough times
        if (repeatCount >= targetRepeats) {
            // Done repeating - advance to next slice based on mode
            int nextSlice = GetNextSlice(currentPlaybackSlice, active_slice_count, knob_stutter, toggle_mode, playbackReverse);
            
            // CRITICAL: Check for read/write conflict
            // If we're about to play the slice we're currently capturing into, skip it
            if (nextSlice == currentCaptureSlice) {
                // Skip ahead one more slice to avoid conflict
                nextSlice = GetNextSlice(nextSlice, active_slice_count, knob_stutter, toggle_mode, playbackReverse);
            }
            
            // Safety check: only advance if next slice has valid content
            // This prevents playing empty slices that haven't been captured yet
            if (sliceLengths[nextSlice] > 0) {
                currentPlaybackSlice = nextSlice;
                playbackPosition = 0;  // CRITICAL: Reset position when changing slices!
            }
            // If next slice is empty, stay on current slice (it will repeat)
            
            // Reset repeat tracking and calculate new behavior for next slice
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
        // Smooth slice length parameter per-sample (prevents clicks when adjusting)
        fonepole(slice_length_samples_smooth, (float)slice_length_samples, 0.0002f);
        
        // Get input (mono, use left channel)
        float input = in[0][i];
        
        float output;
        
        if (!bypass) {
            // Step 3: Read from playback engine
            float wet = PlaybackSlice();
            
            // Step 4: Apply feedback - mix playback back into capture input
            float capture_input = input + (wet * feedback_amount);
            
            // Step 2: Capture with feedback applied
            CaptureSlice(capture_input);
            
            // Dry/wet mix for output
            mix.SetPos(knob_mix);
            output = mix.Process(input, wet);
            
        } else {
            // Bypass - pass input through
            output = input;
        }
        
        // Output to both channels (mono for Phase 1)
        out[0][i] = output;
        out[1][i] = output;
    }
}

// ============================================================================
// MAIN INITIALIZATION
// ============================================================================

int main(void)
{
    // Initialize hardware with CPU boost (480MHz)
    hw.Init(true);
    
    // Seed random number generator for K6 stutter system
    srand(System::GetNow());
    
    // Set audio configuration
    hw.SetAudioBlockSize(512);  // Increased from 256 to reduce clicking from CPU load
    
    // Initialize slice buffer system
    InitializeSliceBuffers();
    
    // Initialize crossfade mixer
    mix.Init();
    
    // Initialize state
    bypass = true;  // Start bypassed for safety
    
    // Initialize control values to safe defaults
    knob_time = 0.0f;
    knob_mix = 0.5f;           // 50/50 mix
    knob_feedback = 0.3f;      // 30% feedback
    knob_slice_count = 0.25f;  // ~4 slices default
    knob_slice_length = 0.4f;  // ~200ms default
    knob_stutter = 0.0f;
    toggle_mode = 0;           // Forward/Forward mode
    
    // Process initial parameters
    ProcessParameters();
    slice_length_samples_smooth = (float)slice_length_samples;  // Initialize smoothed value as float
    
    // Initialize stutter state
    repeatCount = 0;
    targetRepeats = 1;  // Start with no repeats until K6 is adjusted
    
    // Initialize LEDs
    led1.Init(hw.seed.GetPin(Hothouse::LED_1), false);
    led2.Init(hw.seed.GetPin(Hothouse::LED_2), false);
    led1.Set(0.0f);
    led2.Set(0.0f);
    led1.Update();
    led2.Update();
    
    // Start audio
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    
    // Main loop - check for bootloader entry
    while(1)
    {
        // Hold FS1 for 2 seconds to enter bootloader
        if(hw.switches[Hothouse::FOOTSWITCH_1].TimeHeldMs() >= 2000)
        {
            hw.StopAudio();
            hw.StopAdc();
            
            // Flash LEDs to indicate bootloader mode
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
