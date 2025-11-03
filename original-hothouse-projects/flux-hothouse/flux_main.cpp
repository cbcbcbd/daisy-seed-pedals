#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"
#include <stdlib.h>  // For rand() and srand()
#include <cmath>     // For logf()

// Dust is included in daisysp.h - no separate include needed

//
// FLUX v3.0 - Sample & Hold Slicer Delay
// Phase 2 + Phase 5 Lo-Fi Integration (with Dust)
//
// PHASE 1 (COMPLETE):
// ✅ Hardware initialization
// ✅ Bypass switching
// ✅ LED indicators
// ✅ Control processing structure
// ✅ Bootloader entry
// ✅ Slice buffer array structure
// ✅ K2: DRY/WET MIX (0-100%)
// ✅ K3: FEEDBACK (0-100%)
// ✅ K4: SLICE COUNT (1-16)
// ✅ K5: SLICE LENGTH (100-500ms with log curve)
//
// PHASE 2 (COMPLETE):
// ✅ K6: STUTTER control (0-100%)
// ✅ Shuffle probability (0-100% based on K6)
// ✅ Musical subdivision repeats (1x, 2x, 4x, 8x)
// ✅ Random number generator seeded with system time
// ✅ Zero-crossing detection for click-free slicing
// ✅ TOGGLE 1: Capture/Playback Modes
//     - UP: Forward slice sequence, forward playback direction
//     - MIDDLE: Backward slice sequence, reverse playback direction  
//     - DOWN: Forward slice sequence, RANDOM playback direction per slice
// ✅ Variable crossfade system (15% with 5ms minimum)
// ✅ Read/write conflict protection
//
// PHASE 5 (UPDATED - LO-FI INTEGRATION):
// ✅ Custom bit crushing (sample rate reduction + aggressive low-pass filter)
// ✅ Dust noise (sparse impulses for vinyl crackle character)
// ✅ Wobble/flutter (LFO-modulated delay for tape wow/uni-vibe character)
// ✅ Bit crush applied to input BEFORE capture (vintage sampler aesthetic)
// ✅ Wobble applied AFTER dry/wet mix (tape flutter on mixed signal)
// ✅ Dust applied AFTER wobble (vinyl-on-top aesthetic)
// ✅ lofi_bitcrush = 0.0 completely bypasses bit crushing (no overhead)
// ✅ lofi_wobble = 0.0 completely bypasses wobble (no overhead)
// ✅ lofi_noise = 0.0 completely bypasses dust (no overhead)
// ✅ Toggle 3 DOWN enables lo-fi mode (BuzzBox pattern)
// ✅ Touch detection for K3-K6 parameters
//
// Lo-Fi Mode Controls (Toggle 3 DOWN):
// - K3: Wobble (0-100% - tape wow/flutter, uni-vibe character)
// - K4: Dust/Crackle (0-100% - controls both density and mix)
// - K5: Bit Crush (0-100% sample rate reduction)
// - K6: AGE mix (placeholder for future)
//
// Platform: Cleveland Music Co. Hothouse
// Development Stage: DEBUG
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
AdEnv envelope_follower;  // Envelope follower for dynamic slice control

// ============================================================================
// CONTROL STATE
// ============================================================================

bool bypass = true;
bool shift_mode = false;  // Toggle 3 DOWN = lo-fi mode

// Control values (0.0 - 1.0 from knobs)
float knob_time;
float knob_mix;
float knob_feedback;
float knob_slice_count;
float knob_slice_length;
float knob_stutter;

// Toggle values
int toggle_mode;  // Toggle 1 - Capture/Playback mode
int toggle2_mode;  // Toggle 2 - Envelope direction
int prev_toggle3_pos = 0;  // Track Toggle 3 position changes

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

// Envelope Mode control variables
float env_amount;
float env_attack;
float env_release;
float envelope_value = 0.0f;

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
    // CRITICAL: If amount is 0, completely bypass processing
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
    
    // Check if we're in shift mode (Toggle 3 DOWN = Lo-Fi Mode)
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
    
    shift_mode = (toggle3_pos == 2);
    
    // Read Toggle 1 for capture/playback mode
    toggle_mode = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
    
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
    
    if (!shift_mode) {
        // NORMAL MODE - Core slicing parameters
        if (knob_touched[2]) knob_feedback = k3;
        if (knob_touched[3]) knob_slice_count = k4;
        if (knob_touched[4]) knob_slice_length = k5;
        if (knob_touched[5]) knob_stutter = k6;
        
    } else {
        // LO-FI MODE - Degradation effects
        if (knob_touched[2]) lofi_wobble = k3;
        if (knob_touched[3]) lofi_noise = k4;
        if (knob_touched[4]) lofi_bitcrush = k5;  // CCW=0.0 (clean), CW=1.0 (crushed)
        if (knob_touched[5]) lofi_age_mix = k6;
    }
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
    // LED1 - Effect active
    led1.Set(bypass ? 0.0f : 1.0f);
    
    // LED2 - Reserved for future use
    led2.Set(0.0f);
    
    led1.Update();
    led2.Update();
}

void ProcessParameters()
{
    // Map K4 to slice count (1-16)
    active_slice_count = (int)(knob_slice_count * 15.999f) + 1;
    if (active_slice_count < 1) active_slice_count = 1;
    if (active_slice_count > MAX_SLICES) active_slice_count = MAX_SLICES;
    
    // Map K5 to slice length (100-500ms) with logarithmic curve
    float log_knob = logf(1.0f + 9.0f * knob_slice_length) / logf(10.0f);
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
        float dry_input = input;  // Preserve original for dry signal
        
        float output;
        
        if (!bypass) {
            // Apply lo-fi bit crushing to input BEFORE capture
            // This affects what gets captured into slices (vintage sampler aesthetic)
            // CRITICAL: When lofi_bitcrush = 0, CustomBitCrush returns input unchanged
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
    
    bypass = true;
    shift_mode = false;
    
    knob_time = 0.0f;
    knob_mix = 0.5f;
    knob_feedback = 0.3f;
    knob_slice_count = 0.25f;
    knob_slice_length = 0.4f;
    knob_stutter = 0.0f;
    toggle_mode = 0;
    
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
