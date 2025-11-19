#pragma once
#ifndef SIMP_HOTHOUSE_H
#define SIMP_HOTHOUSE_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "../lib/Hothouse.h"

// TODO: Add Q DSP library includes when integrated
// #include "q/pitch/pitch_detector.hpp"
// #include "q/fx/signal_conditioner.hpp"

using namespace daisy;
using namespace daisysp;

class SimpHothouse {
public:
    SimpHothouse() {}
    ~SimpHothouse() {}
    
    void Init(float sampleRate);
    void Process();
    void UpdateControls();
    
private:
    // Hardware
    DaisySeed hw;
    Hothouse controls;
    
    // DSP - Pitch Detection
    // TODO: Add FrequencyDetectorQ when ready
    
    // DSP - String Synthesis
    StringVoice voice1, voice2, voice3, voice4;
    
    // Parameters
    float dryWetMix;
    float sub1Level;
    float sub2Level;
    float brightness;
    float decay;
    float structure;
    
    // State
    float currentFreq;
    float lastFreq;
    bool noteActive;
};

#endif // SIMP_HOTHOUSE_H
