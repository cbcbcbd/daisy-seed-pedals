#include "simp_hothouse.h"

void SimpHothouse::Init(float sampleRate) {
    // Initialize hardware
    hw.Init();
    hw.SetAudioBlockSize(4);
    
    // Initialize Hothouse controls
    controls.Init(&hw);
    
    // Initialize string voices
    voice1.Init(sampleRate);
    voice2.Init(sampleRate);
    voice3.Init(sampleRate);
    voice4.Init(sampleRate);
    
    // Set initial voice parameters
    voice1.SetBrightness(0.7f);
    voice1.SetDamping(0.5f);
    voice1.SetStructure(0.5f);
    
    voice2.SetBrightness(0.6f);
    voice2.SetDamping(0.6f);
    voice2.SetStructure(0.5f);
    
    voice3.SetBrightness(0.5f);
    voice3.SetDamping(0.7f);
    voice3.SetStructure(0.5f);
    
    voice4.SetBrightness(0.8f);
    voice4.SetDamping(0.4f);
    voice4.SetStructure(0.5f);
    
    // Initialize parameters
    dryWetMix = 0.5f;
    sub1Level = 0.4f;
    sub2Level = 0.3f;
    brightness = 0.7f;
    decay = 0.5f;
    structure = 0.5f;
    
    // Initialize state
    currentFreq = 0.0f;
    lastFreq = 0.0f;
    noteActive = false;
    
    // TODO: Initialize pitch detector when ready
}

void SimpHothouse::UpdateControls() {
    controls.ProcessAllControls();
    
    // Read knobs
    dryWetMix = controls.GetKnobValue(Hothouse::KNOB_1);
    sub1Level = controls.GetKnobValue(Hothouse::KNOB_2);
    sub2Level = controls.GetKnobValue(Hothouse::KNOB_3);
    brightness = controls.GetKnobValue(Hothouse::KNOB_4);
    decay = controls.GetKnobValue(Hothouse::KNOB_5);
    structure = controls.GetKnobValue(Hothouse::KNOB_6);
    
    // Update string voice parameters
    voice1.SetBrightness(brightness);
    voice1.SetDamping(decay);
    voice1.SetStructure(structure);
    
    voice2.SetBrightness(brightness * 0.9f);
    voice2.SetDamping(decay * 1.1f);
    voice2.SetStructure(structure);
    
    voice3.SetBrightness(brightness * 0.8f);
    voice3.SetDamping(decay * 1.2f);
    voice3.SetStructure(structure);
    
    voice4.SetBrightness(brightness * 1.1f);
    voice4.SetDamping(decay * 0.9f);
    voice4.SetStructure(structure);
}

void SimpHothouse::Process() {
    // TODO: Implement audio processing
    // 1. Detect pitch
    // 2. Update voice frequencies
    // 3. Trigger voices on note changes
    // 4. Process voices
    // 5. Mix and output
}
