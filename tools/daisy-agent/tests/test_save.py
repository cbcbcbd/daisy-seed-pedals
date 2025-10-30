#!/usr/bin/env python3
"""
Test the save command functionality.
This simulates the save command workflow.
"""

import pyperclip
from pathlib import Path

# Sample code to "save"
SAMPLE_CODE = """// FLUX Phase 2 - Reverse Delay Implementation
#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DelayLine<float, 96000> delay;
bool reverse_mode = false;

void AudioCallback(AudioHandle::InputBuffer in, 
                   AudioHandle::OutputBuffer out, 
                   size_t size) {
    for(size_t i = 0; i < size; i++) {
        float input = in[0][i];
        
        if(reverse_mode) {
            // Reverse delay logic
            float delayed = delay.Read();
            delay.Write(input);
            out[0][i] = delayed;
        } else {
            // Normal delay
            delay.Write(input);
            out[0][i] = delay.Read();
        }
    }
}

int main() {
    // Initialize hardware
    delay.Init();
    return 0;
}
"""

def test_save():
    """Test the save command."""
    
    print("🧪 Testing daisy-agent save command\n")
    
    # Step 1: Put sample code in clipboard
    print("1. Copying sample code to clipboard...")
    pyperclip.copy(SAMPLE_CODE)
    print("   ✓ Sample code copied")
    
    # Step 2: Show the command to run
    print("\n2. Now run this command:")
    print("   " + "=" * 60)
    print("   daisy-agent save --project FLUX --file flux_phase2.cpp \\")
    print("     --description 'Phase 2 with reverse delay'")
    print("   " + "=" * 60)
    
    # Step 3: Explain what should happen
    print("\n3. Expected results:")
    print("   ✓ Reads code from clipboard")
    print("   ✓ Auto-detects type as 'code' (from .cpp extension)")
    print("   ✓ Saves to database with ID")
    print("   ✓ Saves to ~/daisy-agent/projects/FLUX/artifacts/flux_phase2.cpp")
    print("   ✓ Shows success panel with metadata")
    
    # Step 4: Verify file location
    expected_path = Path.home() / "daisy-agent" / "projects" / "FLUX" / "artifacts" / "flux_phase2.cpp"
    print(f"\n4. After running, check file exists:")
    print(f"   {expected_path}")
    
    print("\n" + "=" * 60)
    print("📋 Content in clipboard (ready to save):")
    print("=" * 60)
    print(SAMPLE_CODE[:200] + "...")
    print("\n✅ Clipboard is ready! Run the command above.")


if __name__ == "__main__":
    test_save()
