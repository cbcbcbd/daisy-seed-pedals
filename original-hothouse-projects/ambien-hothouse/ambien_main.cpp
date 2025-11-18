#include "daisy_seed.h"
#include "daisysp.h"
#include "hothouse.h"
#include <stdlib.h>
#include <cmath>

//
// Ambien Delay v1.0 - Ambient Granular Delay with Spectral Phasing
//
// ARCHITECTURE: 4-Voice Spectral Processing with Rhythmic Cascade
// - Captures full-spectrum audio into slices
// - On playback: splits each slice into 3 frequency bands
// - Each band gets Flanger modulation + fixed rhythmic delay
// - Low: No delay (immediate)
// - Mid: 1/4 slice delay (tight cascade)
// - High: 1/2 slice delay (wider cascade)
// - Output: Dry slice + 3 processed bands = 4 voices
//
// PAGE 1 (Toggle 3 UP): Core Delay
//   K1: Master Level (0-200%, unity at 50%)
//   K2: Dry/Wet Mix (0-100%)
//   K3: Feedback (0-100%)
//   K4: Slice Count (1-16)
//   K5: Slice Length (100-500ms)
//   K6: Crossfade Length (0-50%)
//
// PAGE 2 (Toggle 3 MIDDLE): Band Volumes & Filter Q
//   K1: Low Volume (0-200%, unity at 50%)
//   K2: Mid Volume (0-200%, unity at 50%)
//   K3: High Volume (0-200%, unity at 50%)
//   K4: Low Q (0.1-2.0)
//   K5: Mid Q (0.1-2.0)
//   K6: High Q (0.1-2.0)
//
// PAGE 3 (Toggle 3 DOWN): Flanger Settings
//   K1: Low Flanger Depth (0-100%)
//   K2: Mid Flanger Depth (0-100%)
//   K3: High Flanger Depth (0-100%)
//   K4: Low Flanger Rate (0.05-10Hz)
//   K5: Mid Flanger Rate (0.05-10Hz)
//   K6: High Flanger Rate (0.05-10Hz)
//
// TOGGLE 1: Playback Direction
//   UP: Forward / MIDDLE: Reverse / DOWN: Random per slice
//
// TOGGLE 2: Cascade Direction & Original Signal
//   UP: Low→High cascade (Low:0ms, Mid:1/4, High:1/2) + flanged original
//   MIDDLE: High→Low cascade (High:0ms, Mid:1/4, Low:1/2) + flanged original
//   DOWN: Slice cascade only (no original signal mixed in)
//
// Platform: Cleveland Music Co. Hothouse
//

using namespace daisy;
using namespace daisysp;
using namespace clevelandmusicco;

// ============================================================================
// CONSTANTS
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
// SLICE BUFFER SYSTEM (Full-spectrum only)
// ============================================================================

float DSY_SDRAM_BSS sliceBuffers[MAX_SLICES][MAX_SLICE_LENGTH];
int sliceLengths[MAX_SLICES];
float sliceVolumes[MAX_SLICES];  // Volume per slice (decays with each play)

int currentCaptureSlice = 0;
int capturePosition = 0;
bool hasContent = false;

// Zero-crossing detection
bool waitingForZeroCrossing = false;
float previousCaptureSample = 0.0f;
bool hasLeftZero = false;
const int MAX_ZERO_SEARCH = 1000;
int zeroSearchCount = 0;

// Playback state
int currentPlaybackSlice = 0;
int playbackPosition = 0;
bool playbackReverse = false;

// ============================================================================
// DSP MODULES
// ============================================================================

// Frequency splitting filters (guitar-optimized)
Svf lowSplit;    // Low: 80-800Hz
Svf midSplit;    // Mid: 800Hz-1kHz
Svf highSplit;   // High: 1k-4kHz

// Flanger per band for phase modulation
Flanger lowFlanger;
Flanger midFlanger;
Flanger highFlanger;

// Rhythmic offset delays per band (in SDRAM)
DelayLine<float, 24000> DSY_SDRAM_BSS lowRhythmicDelay;   // Max 500ms
DelayLine<float, 24000> DSY_SDRAM_BSS midRhythmicDelay;
DelayLine<float, 24000> DSY_SDRAM_BSS highRhythmicDelay;

// Original signal delay lines (in SDRAM for mixing flanged original with cascading offsets)
DelayLine<float, 24000> DSY_SDRAM_BSS lowOriginalDelay;
DelayLine<float, 24000> DSY_SDRAM_BSS midOriginalDelay;
DelayLine<float, 24000> DSY_SDRAM_BSS highOriginalDelay;

// ============================================================================
// CONTROL STATE
// ============================================================================

bool bypass = true;

// Page 1: Core Delay
float knob_master_level;
float knob_mix;
float knob_feedback;
float knob_slice_count;
float knob_slice_length;
float knob_crossfade;

// Page 2: Band Volumes & Q
float knob_low_volume;
float knob_mid_volume;
float knob_high_volume;
float knob_low_q;
float knob_mid_q;
float knob_high_q;

// Page 3: Phase Modulation
float knob_low_depth;
float knob_mid_depth;
float knob_high_depth;
float knob_low_rate;
float knob_mid_rate;
float knob_high_rate;

// Toggles
int toggle_mode;   // T1: playback direction
int toggle_cascade; // T2: cascade direction (0=Low→High+orig, 1=High→Low+orig, 2=slices only)
int toggle_page;   // T3: page selector (0=Page1, 1=Page2, 2=Page3)

// Touch detection for page switching
float prevKnobValues[6];
bool knob_touched[6];
bool first_start = true;
int prev_toggle_page = 0;

// Processed parameters
int active_slice_count;
float slice_length_ms;
int slice_length_samples;
float slice_length_samples_smooth;
float feedback_amount;
float master_level_amount;
int crossfade_length;

float low_volume, mid_volume, high_volume;
float low_q, mid_q, high_q;
float low_flanger_depth, mid_flanger_depth, high_flanger_depth;
float low_flanger_rate, mid_flanger_rate, high_flanger_rate;

// Rhythmic offset delays (calculated from slice length)
float mid_rhythmic_delay_ms;
float high_rhythmic_delay_ms;

// Original signal delays (depend on cascade direction)
float low_original_delay_ms;
float mid_original_delay_ms;
float high_original_delay_ms;

// ============================================================================
// CONTROL PROCESSING
// ============================================================================

void UpdateControls()
{
    hw.ProcessAllControls();
    
    // Read toggles
    toggle_mode = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1);
    toggle_cascade = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_2);
    toggle_page = hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_3);
    
    // Detect page change
    if (toggle_page != prev_toggle_page && !first_start) {
        prev_toggle_page = toggle_page;
        for (int i = 0; i < 6; i++) {
            knob_touched[i] = false;
            prevKnobValues[i] = hw.GetKnobValue((Hothouse::Knob)i);
        }
    }
    
    // Read knobs and detect movement
    float currentKnobValues[6];
    for (int i = 0; i < 6; i++) {
        currentKnobValues[i] = hw.GetKnobValue((Hothouse::Knob)i);
        if (fabsf(currentKnobValues[i] - prevKnobValues[i]) > 0.02f) {
            knob_touched[i] = true;
            prevKnobValues[i] = currentKnobValues[i];
        }
    }
    
    // Update parameters based on page
    if (toggle_page == 1) {
        // PAGE 2: Band Volumes & Q
        if (knob_touched[0]) knob_low_volume = currentKnobValues[0];
        if (knob_touched[1]) knob_mid_volume = currentKnobValues[1];
        if (knob_touched[2]) knob_high_volume = currentKnobValues[2];
        if (knob_touched[3]) knob_low_q = currentKnobValues[3];
        if (knob_touched[4]) knob_mid_q = currentKnobValues[4];
        if (knob_touched[5]) knob_high_q = currentKnobValues[5];
    } else if (toggle_page == 2) {
        // PAGE 3: Phase Modulation
        if (knob_touched[0]) knob_low_depth = currentKnobValues[0];
        if (knob_touched[1]) knob_mid_depth = currentKnobValues[1];
        if (knob_touched[2]) knob_high_depth = currentKnobValues[2];
        if (knob_touched[3]) knob_low_rate = currentKnobValues[3];
        if (knob_touched[4]) knob_mid_rate = currentKnobValues[4];
        if (knob_touched[5]) knob_high_rate = currentKnobValues[5];
    } else {
        // PAGE 1: Core Delay
        if (knob_touched[0]) knob_master_level = currentKnobValues[0];
        if (knob_touched[1]) knob_mix = currentKnobValues[1];
        if (knob_touched[2]) knob_feedback = currentKnobValues[2];
        if (knob_touched[3]) knob_slice_count = currentKnobValues[3];
        if (knob_touched[4]) knob_slice_length = currentKnobValues[4];
        if (knob_touched[5]) knob_crossfade = currentKnobValues[5];
    }
    
    first_start = false;
}

void UpdateButtons()
{
    if (hw.switches[Hothouse::FOOTSWITCH_1].RisingEdge()) {
        bypass = !bypass;
    }
}

void UpdateLEDs()
{
    led1.Set(bypass ? 0.0f : 1.0f);
    led2.Set(0.0f);
    led1.Update();
    led2.Update();
}

void ProcessParameters()
{
    // Page 1: Core parameters
    active_slice_count = (int)(knob_slice_count * 15.999f) + 1;
    if (active_slice_count < 1) active_slice_count = 1;
    if (active_slice_count > MAX_SLICES) active_slice_count = MAX_SLICES;
    
    float log_knob = logf(1.0f + 9.0f * knob_slice_length) / logf(10.0f);
    slice_length_ms = MIN_SLICE_LENGTH_MS + (log_knob * (MAX_SLICE_LENGTH_MS - MIN_SLICE_LENGTH_MS));
    slice_length_samples = (int)((slice_length_ms / 1000.0f) * SAMPLE_RATE);
    if (slice_length_samples < 1) slice_length_samples = 1;
    if (slice_length_samples > MAX_SLICE_LENGTH) slice_length_samples = MAX_SLICE_LENGTH;
    
    feedback_amount = knob_feedback;
    master_level_amount = knob_master_level * 2.0f;
    
    int currentSliceLen = sliceLengths[currentPlaybackSlice];
    if (currentSliceLen > 0) {
        crossfade_length = (int)(knob_crossfade * 0.5f * currentSliceLen);
        if (crossfade_length < 480) crossfade_length = 480;  // 10ms minimum
        if (crossfade_length * 2 > currentSliceLen) {
            crossfade_length = currentSliceLen / 3;
            if (crossfade_length < 1) crossfade_length = 1;
        }
    } else {
        crossfade_length = 480;  // 10ms minimum
    }
    
    // Page 2: Band volumes and Q
    low_volume = knob_low_volume * 2.0f;
    mid_volume = knob_mid_volume * 2.0f;
    high_volume = knob_high_volume * 2.0f;
    
    low_q = 0.1f + (knob_low_q * 1.9f);
    mid_q = 0.1f + (knob_mid_q * 1.9f);
    high_q = 0.1f + (knob_high_q * 1.9f);
    
    lowSplit.SetRes(low_q);
    midSplit.SetRes(mid_q);
    highSplit.SetRes(high_q);
    
    // Page 3: Flanger settings
    low_flanger_depth = knob_low_depth;
    mid_flanger_depth = knob_mid_depth;
    high_flanger_depth = knob_high_depth;
    
    low_flanger_rate = 0.05f * powf(200.0f, knob_low_rate);
    mid_flanger_rate = 0.05f * powf(200.0f, knob_mid_rate);
    high_flanger_rate = 0.05f * powf(200.0f, knob_high_rate);
    
    lowFlanger.SetLfoDepth(low_flanger_depth);
    lowFlanger.SetLfoFreq(low_flanger_rate);
    
    midFlanger.SetLfoDepth(mid_flanger_depth);
    midFlanger.SetLfoFreq(mid_flanger_rate);
    
    highFlanger.SetLfoDepth(high_flanger_depth);
    highFlanger.SetLfoFreq(high_flanger_rate);
    
    // Calculate rhythmic delays based on slice length
    // Mid band: 1/4 of slice (tight cascade) - minimum 10ms
    // High band: 1/2 of slice (wider cascade) - minimum 20ms
    mid_rhythmic_delay_ms = fmaxf(slice_length_ms / 4.0f, 10.0f);
    high_rhythmic_delay_ms = fmaxf(slice_length_ms / 2.0f, 20.0f);
    
    // Convert to samples for delay lines
    float mid_delay_samples = (mid_rhythmic_delay_ms / 1000.0f) * SAMPLE_RATE;
    float high_delay_samples = (high_rhythmic_delay_ms / 1000.0f) * SAMPLE_RATE;
    
    midRhythmicDelay.SetDelay(mid_delay_samples);
    highRhythmicDelay.SetDelay(high_delay_samples);
    
    // Calculate original signal delays based on cascade direction (Toggle 2)
    if (toggle_cascade == 0) {
        // UP: Low→High cascade
        low_original_delay_ms = 0.0f;
        mid_original_delay_ms = mid_rhythmic_delay_ms;
        high_original_delay_ms = high_rhythmic_delay_ms;
    } else if (toggle_cascade == 1) {
        // MIDDLE: High→Low cascade (reversed)
        high_original_delay_ms = 0.0f;
        mid_original_delay_ms = mid_rhythmic_delay_ms;
        low_original_delay_ms = high_rhythmic_delay_ms;
    } else {
        // DOWN: No original signal (delays don't matter)
        low_original_delay_ms = 0.0f;
        mid_original_delay_ms = 0.0f;
        high_original_delay_ms = 0.0f;
    }
    
    // Set original signal delay lines
    float low_orig_samples = (low_original_delay_ms / 1000.0f) * SAMPLE_RATE;
    float mid_orig_samples = (mid_original_delay_ms / 1000.0f) * SAMPLE_RATE;
    float high_orig_samples = (high_original_delay_ms / 1000.0f) * SAMPLE_RATE;
    
    lowOriginalDelay.SetDelay(low_orig_samples);
    midOriginalDelay.SetDelay(mid_orig_samples);
    highOriginalDelay.SetDelay(high_orig_samples);
}

// ============================================================================
// PLAYBACK DIRECTION
// ============================================================================

int GetNextSlice(int current, int sliceCount, int mode, bool& reverseFlag)
{
    int nextSlice;
    
    if (mode == 1) {
        // MIDDLE: Reverse
        nextSlice = (current - 1 + sliceCount) % sliceCount;
        reverseFlag = true;
    } else if (mode == 2) {
        // DOWN: Random direction per slice
        nextSlice = (current + 1) % sliceCount;
        reverseFlag = (rand() % 2) == 0;
    } else {
        // UP: Forward
        nextSlice = (current + 1) % sliceCount;
        reverseFlag = false;
    }
    
    return nextSlice;
}

// ============================================================================
// SLICE BUFFER INITIALIZATION
// ============================================================================

void InitializeSliceBuffers()
{
    for (int slice = 0; slice < MAX_SLICES; slice++) {
        sliceLengths[slice] = 0;
        sliceVolumes[slice] = 1.0f;  // Full volume initially
        for (int sample = 0; sample < MAX_SLICE_LENGTH; sample++) {
            sliceBuffers[slice][sample] = 0.0f;
        }
    }
    
    currentCaptureSlice = 0;
    capturePosition = 0;
    currentPlaybackSlice = 0;
    playbackPosition = 0;
    hasContent = false;
    
    waitingForZeroCrossing = false;
    previousCaptureSample = 0.0f;
    hasLeftZero = false;
    zeroSearchCount = 0;
}

// ============================================================================
// SLICE CAPTURE (Full-spectrum)
// ============================================================================

void CaptureSlice(float input)
{
    int target_length = (int)slice_length_samples_smooth;
    if (target_length > MAX_SLICE_LENGTH) {
        target_length = MAX_SLICE_LENGTH;
    }
    
    if (waitingForZeroCrossing) {
        zeroSearchCount++;
        
        const float ZERO_THRESHOLD = 0.01f;
        
        if (!hasLeftZero) {
            if (fabsf(input) > ZERO_THRESHOLD) {
                hasLeftZero = true;
                previousCaptureSample = input;
            }
        } else {
            bool crossingDetected = (previousCaptureSample > 0.0f && input <= 0.0f) || 
                                   (previousCaptureSample < 0.0f && input >= 0.0f);
            
            if (crossingDetected || zeroSearchCount >= MAX_ZERO_SEARCH) {
                sliceLengths[currentCaptureSlice] = capturePosition;
                sliceVolumes[currentCaptureSlice] = 1.0f;  // Reset to full volume on new capture
                
                currentCaptureSlice++;
                if (currentCaptureSlice >= active_slice_count) {
                    currentCaptureSlice = 0;
                }
                
                capturePosition = 0;
                waitingForZeroCrossing = false;
                hasLeftZero = false;
                zeroSearchCount = 0;
                hasContent = true;
            }
        }
        
        previousCaptureSample = input;
        
        if (capturePosition < MAX_SLICE_LENGTH) {
            sliceBuffers[currentCaptureSlice][capturePosition] = input;
            capturePosition++;
        }
        
        return;
    }
    
    sliceBuffers[currentCaptureSlice][capturePosition] = input;
    capturePosition++;
    
    if (capturePosition >= target_length) {
        waitingForZeroCrossing = true;
        hasLeftZero = false;
        zeroSearchCount = 0;
        previousCaptureSample = input;
    }
}

// ============================================================================
// 4-VOICE PLAYBACK (Dry + 3 Phased Bands)
// ============================================================================

float PlaybackSlice()
{
    if (!hasContent || sliceLengths[currentPlaybackSlice] <= 0) {
        return 0.0f;
    }
    
    int sliceLength = sliceLengths[currentPlaybackSlice];
    
    // Calculate read position
    int readPosition;
    if (playbackReverse) {
        readPosition = sliceLength - 1 - playbackPosition;
        if (readPosition < 0) readPosition = 0;
    } else {
        readPosition = playbackPosition;
    }
    
    if (readPosition >= sliceLength) readPosition = sliceLength - 1;
    if (readPosition < 0) readPosition = 0;
    
    // Read dry sample
    float drySample = sliceBuffers[currentPlaybackSlice][readPosition];
    
    // Split into 3 frequency bands
    lowSplit.Process(drySample);
    midSplit.Process(drySample);
    highSplit.Process(drySample);
    
    float lowBand = lowSplit.Low();
    float midBand = midSplit.Band();
    float highBand = highSplit.High();
    
    // Apply Flanger to each band (if volume > 0)
    float lowProcessed = 0.0f;
    float midProcessed = 0.0f;
    float highProcessed = 0.0f;
    
    if (low_volume > 0.01f) {
        lowProcessed = lowFlanger.Process(lowBand) * low_volume;
    }
    
    if (mid_volume > 0.01f) {
        // Mid band: Flanger + rhythmic delay (1/4 slice)
        float flanged = midFlanger.Process(midBand);
        midRhythmicDelay.Write(flanged);
        midProcessed = midRhythmicDelay.Read() * mid_volume;
    }
    
    if (high_volume > 0.01f) {
        // High band: Flanger + rhythmic delay (1/2 slice)
        float flanged = highFlanger.Process(highBand);
        highRhythmicDelay.Write(flanged);
        highProcessed = highRhythmicDelay.Read() * high_volume;
    }
    
    // Apply equal-power crossfades at slice boundaries (reduces clicks)
    float fadeEnvelope = 1.0f;
    
    if (playbackPosition < crossfade_length) {
        // Fade in with sqrt for equal-power
        fadeEnvelope = sqrtf((float)playbackPosition / (float)crossfade_length);
    }
    
    int fadeOutStart = sliceLength - crossfade_length;
    if (fadeOutStart > 0 && playbackPosition >= fadeOutStart) {
        int fadeOutPos = playbackPosition - fadeOutStart;
        // Fade out with sqrt for equal-power
        float fadeOutEnvelope = sqrtf(1.0f - ((float)fadeOutPos / (float)crossfade_length));
        if (fadeOutEnvelope < fadeEnvelope) {
            fadeEnvelope = fadeOutEnvelope;
        }
    }
    
    // Mix 4 voices: Dry + 3 processed bands (Flanger + rhythmic offsets)
    float output = (drySample + lowProcessed + midProcessed + highProcessed) * fadeEnvelope;
    
    // Apply slice volume (decays with each playback)
    output *= sliceVolumes[currentPlaybackSlice];
    
    // Advance playback
    playbackPosition++;
    
    if (playbackPosition >= sliceLength) {
        playbackPosition = 0;
        
        // Decay slice volume based on feedback amount
        // High feedback = slow decay, Low feedback = fast decay
        // Formula: volume *= (0.5 + 0.45 * feedback)
        // At 0% feedback: volume *= 0.5 (decays quickly - 50% per cycle)
        // At 100% feedback: volume *= 0.95 (very slow decay - 5% per cycle)
        float decay_factor = 0.5f + (0.45f * feedback_amount);
        sliceVolumes[currentPlaybackSlice] *= decay_factor;
        
        // If volume drops below threshold, consider it silent
        if (sliceVolumes[currentPlaybackSlice] < 0.001f) {
            sliceVolumes[currentPlaybackSlice] = 0.0f;
        }
        
        int nextSlice = GetNextSlice(currentPlaybackSlice, active_slice_count, toggle_mode, playbackReverse);
        
        if (nextSlice == currentCaptureSlice) {
            nextSlice = GetNextSlice(nextSlice, active_slice_count, toggle_mode, playbackReverse);
        }
        
        if (sliceLengths[nextSlice] > 0) {
            currentPlaybackSlice = nextSlice;
            playbackPosition = 0;
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
        float output;
        
        if (!bypass) {
            float wet = PlaybackSlice();
            
            // Process original signal through spectral flanging (if Toggle 2 != DOWN)
            float original_flanged = 0.0f;
            if (toggle_cascade != 2) {
                // Split original into 3 frequency bands
                lowSplit.Process(input);
                midSplit.Process(input);
                highSplit.Process(input);
                
                float lowBandOrig = lowSplit.Low();
                float midBandOrig = midSplit.Band();
                float highBandOrig = highSplit.High();
                
                // Apply Flangers to each band
                float lowFlangedOrig = lowFlanger.Process(lowBandOrig);
                float midFlangedOrig = midFlanger.Process(midBandOrig);
                float highFlangedOrig = highFlanger.Process(highBandOrig);
                
                // Apply cascading delays to create rhythmic offset
                lowOriginalDelay.Write(lowFlangedOrig * low_volume);
                midOriginalDelay.Write(midFlangedOrig * mid_volume);
                highOriginalDelay.Write(highFlangedOrig * high_volume);
                
                float lowDelayed = lowOriginalDelay.Read();
                float midDelayed = midOriginalDelay.Read();
                float highDelayed = highOriginalDelay.Read();
                
                // Sum the cascaded bands
                original_flanged = lowDelayed + midDelayed + highDelayed;
            }
            
            // Feedback disabled (was causing oscillation)
            // TODO: Need proper feedback implementation with gain control
            float capture_input = input;
            CaptureSlice(capture_input);
            
            // Combine sliced wet signal with original flanged signal
            float combined_wet = wet + original_flanged;
            
            // Constant-power dry/wet mix (prevents volume dip at 50%)
            // Uses sqrt for equal-power curve
            float wet_level = sqrtf(knob_mix);
            float dry_level = sqrtf(1.0f - knob_mix);
            output = (input * dry_level) + (combined_wet * wet_level);
            
            // Apply master level
            output *= master_level_amount;
        } else {
            output = input;
        }
        
        out[0][i] = output;
        out[1][i] = output;
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(void)
{
    hw.Init(true);
    srand(System::GetNow());
    hw.SetAudioBlockSize(512);
    
    InitializeSliceBuffers();
    
    // Initialize DSP modules
    lowSplit.Init(SAMPLE_RATE);
    lowSplit.SetFreq(800.0f);
    lowSplit.SetRes(0.1f);
    
    midSplit.Init(SAMPLE_RATE);
    midSplit.SetFreq(900.0f);
    midSplit.SetRes(0.1f);
    
    highSplit.Init(SAMPLE_RATE);
    highSplit.SetFreq(1000.0f);
    highSplit.SetRes(0.1f);
    
    lowFlanger.Init(SAMPLE_RATE);
    lowFlanger.SetFeedback(0.5f);
    
    midFlanger.Init(SAMPLE_RATE);
    midFlanger.SetFeedback(0.5f);
    
    highFlanger.Init(SAMPLE_RATE);
    highFlanger.SetFeedback(0.5f);
    
    lowRhythmicDelay.Init();
    midRhythmicDelay.Init();
    highRhythmicDelay.Init();
    
    lowOriginalDelay.Init();
    midOriginalDelay.Init();
    highOriginalDelay.Init();
    
    // Initialize controls to safe defaults
    bypass = true;
    knob_master_level = 0.5f;  // Unity
    knob_mix = 0.5f;
    knob_feedback = 0.3f;
    knob_slice_count = 0.25f;
    knob_slice_length = 0.4f;
    knob_crossfade = 0.3f;
    
    knob_low_volume = 0.5f;    // Unity
    knob_mid_volume = 0.5f;
    knob_high_volume = 0.5f;
    knob_low_q = 0.0f;         // Minimal Q
    knob_mid_q = 0.0f;
    knob_high_q = 0.0f;
    
    knob_low_depth = 0.0f;     // No phase delay
    knob_mid_depth = 0.0f;
    knob_high_depth = 0.0f;
    knob_low_rate = 0.2f;      // ~1Hz
    knob_mid_rate = 0.2f;
    knob_high_rate = 0.2f;
    
    toggle_mode = 0;
    toggle_cascade = 0;
    toggle_page = 0;
    
    ProcessParameters();
    slice_length_samples_smooth = (float)slice_length_samples;
    
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
        // Hothouse DFU entry - QSPI compatible
        hw.CheckResetToBootloader();
        
        System::Delay(100);
    }
}
