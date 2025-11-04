# Hothouse Pedal Development Package - Setup Instructions

This package provides everything you need to start building audio effect pedals for the Cleveland Music Co. Hothouse platform using Claude AI.

---

## Package Contents

1. **HOTHOUSE_QUICK_START_PROMPT.md** - Complete standalone prompt for immediate use
2. **SESSION_STARTER_TEMPLATE.md** - Structured template for Claude Projects
3. **HOTHOUSE_ESSENTIALS.json** - Core API reference and gotchas
4. **SETUP_INSTRUCTIONS.md** - This file

---

## Two Ways to Use This Package

### Option A: Quick Start (No Setup Required)

**Best for**: Quick experiments, one-off projects, learning

**How to use**:
1. Open a new Claude conversation
2. Copy and paste the entire contents of **HOTHOUSE_QUICK_START_PROMPT.md**
3. Add your project description
4. Start developing!

**Pros**: Immediate, no configuration
**Cons**: Must paste full prompt each session, no persistent knowledge

---

### Option B: Claude Project with Knowledge Base (Recommended)

**Best for**: Ongoing development, multiple projects, serious work

**Setup Steps**:

1. **Create a Claude Project**:
   - In Claude.ai, click "Projects" in the sidebar
   - Click "+ New Project"
   - Name it "Hothouse Pedal Development"

2. **Add Knowledge Files**:
   - Click "Add content" in your new project
   - Upload these files from your package:
     - `HOTHOUSE_ESSENTIALS.json`
     - `SESSION_STARTER_TEMPLATE.md`
   
   - **Optional but highly recommended** - Also add these if you have them:
     - `claude-hothouse-ref.json` (complete API reference)
     - `claude-daisysp-ref.json` (DaisySP library reference)
     - `claude-rules-v2.0.json` (behavioral rules)
     - `Funbox_to_Hothouse_Conversion_Guide_v6_md.md` (conversion guide)
     - `Prompts_for_DSP_v8_0.md` (DSP patterns)

3. **Set Project Instructions** (Optional):
   - In project settings, add this as custom instructions:
   ```
   This project is for developing audio effect pedals for the Hothouse platform.
   Always use GUIDED mode by default (ask before implementing).
   Report token status after every response.
   Search project knowledge files before answering questions.
   ```

4. **Start a Conversation**:
   - Click "New chat" within your project
   - Copy the **SESSION_STARTER_TEMPLATE.md** content
   - Customize the SESSION INFO section
   - Paste and start developing!

**Pros**: 
- Knowledge persists across sessions
- Claude automatically accesses reference files
- More efficient token usage
- Can upload your own pedal code for reference

**Cons**: 
- Requires initial setup
- Limited to 5 projects on Free tier (unlimited on Pro)

---

## Understanding the Files

### HOTHOUSE_QUICK_START_PROMPT.md
- **What**: Complete reference in a single document
- **Contains**: API reference, gotchas, code examples, workflow
- **Size**: ~3,000 tokens
- **Use when**: You want everything in one place, quick prototyping

### SESSION_STARTER_TEMPLATE.md  
- **What**: Structured template that references knowledge files
- **Contains**: Mode definitions, examples, behavioral rules
- **Size**: ~1,500 tokens
- **Use when**: Working in a Claude Project with knowledge base

### HOTHOUSE_ESSENTIALS.json
- **What**: Machine-readable reference in JSON format
- **Contains**: API specs, gotchas, templates, conversion guide
- **Size**: ~2,000 tokens
- **Use when**: Added to Claude Project knowledge base

---

## Typical Workflow

### Starting a New Pedal

1. Open your Hothouse project (or start fresh conversation)
2. Paste session starter template
3. Set Mode: **EXPANSION**
4. Describe what you want to build:
   ```
   I want to create a delay pedal with:
   - Time: 50ms to 2 seconds
   - Feedback: 0-95%
   - Mix: dry/wet blend
   - Modulation: optional chorus effect
   - Tap tempo on FS2
   ```
5. Claude will ask clarifying questions
6. Once requirements are clear, Claude implements incrementally

### Converting from Funbox

1. Open your Hothouse project
2. Paste session starter template
3. Set Mode: **REPLICATION**
4. Provide original Funbox code (paste or upload file)
5. Claude converts following exact conversion guide
6. Test on hardware
7. Switch to DEBUG mode if issues arise

### Continuing Development

1. Open your Hothouse project
2. Paste session starter template  
3. Set Mode: **GUIDED**
4. Specify working code file and what to add next
5. Claude asks before implementing anything
6. Approve changes, test incrementally

### Debugging Issues

1. Open your Hothouse project
2. Paste session starter template
3. Set Mode: **DEBUG**
4. Describe the issue and provide relevant code
5. Claude identifies problems and suggests minimal fixes
6. Test each fix before proceeding

---

## Best Practices

### DO:
✅ Always specify the Mode (GUIDED, EXPANSION, etc.)
✅ Include "Working Code" filename when continuing work
✅ Test on hardware after each feature addition
✅ Use REPLICATION mode when converting existing code
✅ Ask Claude to explain anything unclear
✅ Report when something works or doesn't work
✅ Save working versions frequently

### DON'T:
❌ Skip the session starter template
❌ Let Claude implement without approval in GUIDED mode
❌ Modify bootloader code without checking
❌ Test only in simulation (always verify on hardware)
❌ "Fix" things in REPLICATION mode without testing first
❌ Add multiple complex features at once
❌ Forget to add hothouse.cpp to Makefile

---

## Getting Additional Files

The complete knowledge base includes additional files not in this basic package:

**Where to find them**:
- **Hothouse Examples**: https://github.com/clevelandmusicco/HothouseExamples
- **DaisySP Docs**: https://electro-smith.github.io/DaisySP/
- **libDaisy Docs**: https://electro-smith.github.io/libDaisy/
- **Forum**: https://forum.electro-smith.com/

**Essential files to grab**:
- `hothouse.cpp` and `hothouse.h` from HothouseExamples (required for every project)
- Working example projects for reference
- DaisySP header files for DSP module documentation

---

## Token Management

Claude sessions have token limits. Here's how to manage them:

**Token Zones** (Claude reports after each response):
- 🟢 GREEN (>60% remaining): Normal operation
- 🟡 YELLOW (40-60%): Be concise, focus on implementation  
- 🟠 ORANGE (20-40%): Wrap up current task
- 🔴 RED (<20%): Emergency mode, prepare handoff

**Token-saving tips**:
- Use Claude Projects (knowledge base uses fewer tokens)
- Reference files instead of pasting full code repeatedly
- Break large projects into multiple sessions
- Use concise descriptions in yellow/orange zones

---

## Troubleshooting

**"Claude doesn't follow the template"**
- Make sure you pasted the complete template
- Verify Mode is specified clearly
- Remind Claude if it starts implementing without asking

**"Claude doesn't find knowledge files"**
- Check files are uploaded to the Project (not just attached to one message)
- Try explicitly saying "Search HOTHOUSE_ESSENTIALS.json for [topic]"
- Verify you're working in the Project, not a standalone chat

**"Code doesn't compile"**
- Verify hothouse.cpp is in Makefile CPP_SOURCES
- Check that hothouse.cpp/h match (same version)
- Ensure libDaisy and DaisySP are built first
- Look for typos in enum names (TOGGLESWITCH not TOGGLE)

**"Controls don't work"**
- Verify ProcessAllControls() is called in audio callback
- Check enum names match Hothouse API exactly
- Test on hardware (simulation may not show control issues)

**"Getting CPU overload"**
- Try without boost first: hw.Init(false)
- Reduce audio block size
- Disable features one-by-one to find culprit
- Consider simpler algorithms for complex combinations

---

## Example Session Start

Here's what a complete session start looks like:

```
Review: HOTHOUSE_ESSENTIALS.json
Review: SESSION_STARTER_TEMPLATE.md

Mode: EXPANSION
Project: Shimmer Reverb  
Platform: Hothouse
Current Work: Creating initial requirements for shimmer reverb pedal

I want to create a shimmer reverb pedal with:
- Reverb with octave-up shimmer effect
- Decay time: 0.5 to 10 seconds
- Shimmer amount: 0-100% (mix of octave)
- Tone control: 200Hz to 5kHz (affects reverb tail)
- Mix: Dry/wet blend
- Toggle 1: Reverb algorithm (Hall, Plate, Room)
- Toggle 2: Shimmer mode (Off, Single octave, Dual octave)
- FS1: Bypass
- FS2: Freeze (hold current reverb tail)

Questions:
1. Should the octave be pre-reverb or post-reverb?
2. Do you want modulation on the reverb tail?
3. Any specific algorithm preferences?
```

Claude will then ask clarifying questions before starting implementation.

---

## Support and Resources

**Documentation**:
- Hothouse: https://github.com/clevelandmusicco/HothouseExamples
- DaisySP: https://electro-smith.github.io/DaisySP/
- libDaisy: https://electro-smith.github.io/libDaisy/

**Community**:
- Forum: https://forum.electro-smith.com/
- Discord: Available through forum

**Tools**:
- Web Programmer: https://electro-smith.github.io/Programmer/
- DFU Util: For command-line programming

---

## Ready to Build!

You now have everything needed to start building Hothouse pedals with Claude. 

**Quick checklist**:
- [ ] Chose Quick Start or Project approach
- [ ] Have hothouse.cpp/h files ready
- [ ] Understand the Mode system
- [ ] Know how to start a session
- [ ] Ready to test on hardware

**Start building and have fun creating amazing guitar pedals!**
