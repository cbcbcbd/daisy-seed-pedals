# Hothouse Pedal Development - Session Starter Template

**For use with Claude Projects containing knowledge files. Copy and paste this at the start of every session.**

---

## LOAD PROTOCOL

```
Review: claude-hothouse-ref.json
Review: claude-daisysp-ref.json  
Review: claude-rules-v2.0.json
Review: Funbox_to_Hothouse_Conversion_Guide_v6_md.md
Review: Prompts_for_DSP_v8_0.md
```

---

## SESSION INFO

**Mode:** [GUIDED / EXPANSION / REPLICATION / DEBUG / OPTIMIZATION]

**Project:** [Your Project Name]

**Platform:** Hothouse

**Current Work:** [Brief description of what you're working on]

**Working Code:** [Filename of last working version, if applicable]

---

## Mode Definitions

### GUIDED (Default for Development)
- **Purpose**: Controlled development - ask before implementing
- **Behavior**: Present plan, wait for approval, implement incrementally
- **Use when**: Building new features, making changes to working code

### EXPANSION  
- **Purpose**: Adding new features with full requirements gathering
- **Behavior**: Don't start coding until ALL requirements are gathered
- **Use when**: Adding major new functionality

### REPLICATION
- **Purpose**: Converting existing code exactly - zero changes
- **Behavior**: Exact copy only, ask before "fixing" anything
- **Use when**: Converting Funbox to Hothouse, porting existing pedals

### DEBUG
- **Purpose**: Fix only what's broken - minimal changes
- **Behavior**: Small fixes only, test each change incrementally
- **Use when**: Something broke and needs fixing

### OPTIMIZATION
- **Purpose**: Careful improvements to working code
- **Behavior**: Small increments, test each change, ask before major refactoring
- **Use when**: Code works but needs performance/clarity improvements

---

## Examples

### Example 1: Starting New Pedal Design
```
Review: claude-hothouse-ref.json
Review: claude-daisysp-ref.json
Review: claude-rules-v2.0.json
Review: Prompts_for_DSP_v8_0.md

Mode: EXPANSION
Project: Shimmer Reverb
Platform: Hothouse
Current Work: Gathering requirements for shimmer reverb with octave up
```

### Example 2: Converting Funbox Pedal
```
Review: claude-hothouse-ref.json
Review: Funbox_to_Hothouse_Conversion_Guide_v6_md.md
Review: claude-rules-v2.0.json

Mode: REPLICATION
Project: Venus Spectral Reverb
Platform: Hothouse
Current Work: Converting Funbox Venus to Hothouse - Phase 1
Working Code: [Paste original Funbox code or attach file]
```

### Example 3: Continuing Development
```
Review: claude-hothouse-ref.json
Review: claude-daisysp-ref.json
Review: claude-rules-v2.0.json

Mode: GUIDED
Project: FLUX Modulated Slicer
Platform: Hothouse
Current Work: Implementing Phase 5 Lo-Fi mode with bit crushing
Working Code: flux_hothouse_v4.cpp
```

### Example 4: Debugging Session
```
Review: claude-hothouse-ref.json
Review: claude-rules-v2.0.json

Mode: DEBUG
Project: BuzzBox Fuzz
Platform: Hothouse
Current Work: Fixing volume drop when FS2 engaged
Working Code: buzzbox_hothouse.cpp (last working version)

Issue: Volume drops significantly when footswitch 2 is pressed
```

### Example 5: Optimizing Working Code
```
Review: claude-hothouse-ref.json
Review: claude-rules-v2.0.json

Mode: OPTIMIZATION
Project: Earth Reverbscape
Platform: Hothouse
Current Work: Reducing CPU usage in reverb processing
Working Code: earth_hothouse.cpp (current working version)
```

---

## Critical Behavioral Rules

### Token Reporting (Required Every Response)
Format: `Tokens: X used | Y remaining | Z% remaining (ZONE) 🟢`

**Zones:**
- 🟢 GREEN (>60% remaining): Normal operation
- 🟡 YELLOW (40-60%): Concise responses
- 🟠 ORANGE (20-40%): Wrap-up mode
- 🔴 RED (<20%): Emergency mode

### GUIDED Mode Rules (Most Important)
1. **ASK before implementing ANYTHING**
2. Present plan and wait for explicit approval
3. Report tokens after every response
4. Never assume requirements or start coding without approval

### General Rules
- Search project knowledge files when referencing documentation
- Never modify bootloader entry code
- Always test incrementally
- Preserve working DSP algorithms exactly
- Document all assumptions

---

## What to Include in Your First Message

After pasting this template, add:

**For new pedals:**
```
I want to create a [effect type] pedal with:
- [Feature 1]
- [Feature 2]
- [Feature 3]

Control layout ideas:
- Knob 1: [function]
- Knob 2: [function]
[etc.]
```

**For conversions:**
```
I'm converting [project name] from Funbox to Hothouse.

[Paste original code or attach file]

Or: "The code is in the attached file: [filename]"
```

**For debugging:**
```
I'm experiencing [specific issue description].

Current behavior: [what's happening]
Expected behavior: [what should happen]

Relevant code section:
[paste code]
```

---

## Notes

- **Claude will automatically search** the knowledge files listed in LOAD PROTOCOL
- **Mode is mandatory** - determines how Claude behaves
- **Platform: Hothouse** ensures correct API usage
- **Working Code** helps Claude find the right starting point for debugging/optimization
- Always include specific details about what you want to accomplish

---

## Ready to Start

Paste this template, add your project details, and let's build your Hothouse pedal!
