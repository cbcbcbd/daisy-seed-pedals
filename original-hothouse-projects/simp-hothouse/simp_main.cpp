#include "simp_hothouse.h"

SimpHothouse simp;

void AudioCallback(AudioHandle::InputBuffer in, 
                   AudioHandle::OutputBuffer out, 
                   size_t size) {
    simp.Process();
    
    // TODO: Implement audio processing loop
    // For now, pass through
    for (size_t i = 0; i < size; i++) {
        out[0][i] = in[0][i];  // Left
        out[1][i] = in[1][i];  // Right
    }
}

int main(void) {
    // Initialize at 48kHz
    simp.Init(48000.0f);
    
    // Start audio
    simp.hw.StartAudio(AudioCallback);
    
    // Main loop
    while(1) {
        simp.UpdateControls();
        System::Delay(10);  // 10ms control rate
    }
}
