# Product Requirements Document: Daisy Agent

**Version:** 2.0  
**Date:** October 17, 2025  
**Status:** Phase 0 Complete - In Active Development  
**Project:** daisy-agent (formerly "Hothouse Session Orchestration Agent")

---

## Executive Summary

Daisy Agent is a Python-based session orchestration tool for Daisy Seed embedded DSP development. It compresses project knowledge (70-85% token reduction), generates optimized session handoffs, and will eventually enforce development workflow discipline across Claude sessions.

**Target Users:** Developers working on Daisy Seed firmware (Hothouse, Funbox, Terrarium, and other platforms)  
**Primary Goal:** Eliminate session startup overhead and enable seamless multi-session workflows  
**Current Status:** Phase 0 MVP complete and functional

**Key Metrics (Target):**
- 70-85% reduction in session startup tokens
- <2 seconds for context compression
- Zero manual context management
- Unlimited effective context across sessions

---

## Revision History

| Version | Date | Changes | Author |
|---------|------|---------|--------|
| 1.0 | Oct 16, 2025 | Initial PRD with full architecture | Design Team |
| 2.0 | Oct 17, 2025 | Phase 0 implementation, renamed to daisy-agent, mode system updated | C. Brandt |

**Major Changes in v2.0:**
- Renamed from "Hothouse" to "Daisy" Agent (platform-agnostic)
- Changed "Stage" to "Mode" with GUIDED/HANDOFF as primary modes
- Phase 0 MVP completed and tested
- Working implementation with real project files
- Virtual environment setup documented

---

## Problem Statement (Updated)

### Scope Expansion

**Original Scope:** Tool for Hothouse platform development  
**Updated Scope:** Universal tool for all Daisy Seed platforms (Hothouse, Funbox, Terrarium, future platforms)

### Platform Abstraction Required

The agent must:
- Support multiple hardware platforms with different APIs
- Parse platform `.h` header files OR `.json` knowledge files
- Abstract platform-specific knowledge from core agent logic
- Enable users to add new platforms easily

### Current Pain Points (Unchanged)

1. **Token Budget Exhaustion:** Session startup consumes 20-30K tokens (10-15% of budget)
2. **Manual Handoffs:** Copying context between sessions is error-prone
3. **Mode Discipline:** Claude defaults to "helpful" instead of "ask first"
4. **Lost Context:** No persistent memory across sessions

---

## Solution Overview

### What We've Built (Phase 0)

A command-line tool that:
1. **Indexes project knowledge** into SQLite database
2. **Compresses context** using hash-based semantic search
3. **Generates handoff documents** with compressed context embedded
4. **Saves and copies handoffs** for easy session transitions

### Architecture Decisions Made

| Decision | Rationale |
|----------|-----------|
| **Python 3.13+** | User's installed version, good ecosystem |
| **Virtual environment** | Required by Homebrew Python, isolates dependencies |
| **SQLite** | Built-in, no external database needed |
| **Hash-based embeddings** | Fast (<2ms), no ML dependencies for MVP |
| **Click framework** | Industry-standard Python CLI library |
| **Rich library** | Beautiful terminal output |

---

## Terminology Changes

### Mode System (formerly "Stage")

**Previous terminology caused confusion.** Updated as follows:

| Old Term | New Term | Purpose |
|----------|----------|---------|
| EXPANSION | **GUIDED** | Default mode: ask questions, wait for approval, careful implementation |
| SUMMARIZE | **HANDOFF** | Session wrap-up mode: prepare handoff docs, summarize progress |
| INITIATE | *(Deferred)* | May add later if needed |
| DEBUG | *(Deferred)* | May add later if needed |
| REPLICATION | *(Deferred)* | May add later if needed |

**Rationale:** User primarily uses two modes in practice. GUIDED better describes the "ask-first" behavior.

---

## Functional Requirements

### FR1: Context Compression Engine ✅ COMPLETE

**Status:** Phase 0 MVP implemented and tested

**What It Does:**
- Parses JSON documents (claude-rules.json, platform refs, etc.)
- Chunks documents by top-level keys (semantic boundaries)
- Creates hash-based embeddings for similarity matching
- Stores chunks in SQLite with token counts
- Retrieves relevant chunks based on query
- Generates compressed context within token budget

**Current Performance:**
- 3 files indexed: 31 chunks, 4,306 tokens
- Compression: 4,306 → 1,826 tokens (57.6%)
- Search time: <2ms per query
- Zero external dependencies beyond stdlib + installed packages

**Acceptance Criteria:**
- ✅ Can index JSON files
- ✅ Estimates tokens conservatively
- ✅ Retrieves relevant chunks
- ✅ Generates compressed output
- ⏳ 70%+ compression (will improve with more documents)

**Known Limitations:**
- Hash-based search is basic (exact keyword match)
- No relevance scoring yet
- Compression ratio depends on knowledge base size
- Only supports JSON files currently

### FR2: Handoff Document Generation ✅ COMPLETE

**Status:** Phase 0 MVP implemented and tested

**What It Does:**
- Accepts project info (name, current work, working file)
- Generates structured handoff document
- Embeds compressed context directly (no "Review:" references)
- Saves to timestamped file in ~/daisy-agent/handoffs/
- Copies to clipboard automatically
- Uses HANDOFF mode in generated document

**Usage Modes:**
1. **Interactive:** `daisy-agent handoff` (prompts for info)
2. **CLI:** `daisy-agent handoff --project FLUX --current "..." --working-file flux.cpp`

**Acceptance Criteria:**
- ✅ Interactive prompting works
- ✅ CLI flags work
- ✅ Saves to timestamped file
- ✅ Copies to clipboard
- ✅ Embeds compressed context
- ✅ Uses consistent format

**Output Format:**
```markdown
# Session Handoff: PROJECT - YYYY-MM-DD HH:MM

## SESSION INFO
Mode: HANDOFF
Project: [name]
Current Work: [description]
Working Code: [optional filename]

## COMPRESSED CONTEXT (X,XXX tokens)
[Compressed relevant knowledge]

## NEXT SESSION
When starting the next session, use GUIDED mode for continued development.
```

### FR3: Knowledge Base Management ✅ COMPLETE

**Status:** Phase 0 MVP implemented

**Commands:**
- `daisy-agent init` - Initialize agent environment
- `daisy-agent index <file>` - Index a knowledge file
- `daisy-agent stats` - Show database statistics
- `daisy-agent compress <query>` - Test compression

**File Locations:**
- **Agent home:** `~/daisy-agent/`
- **Database:** `~/daisy-agent/knowledge.db`
- **Handoffs:** `~/daisy-agent/handoffs/`
- **Platforms:** `~/daisy-agent/platforms/` (future)

**Acceptance Criteria:**
- ✅ Creates directory structure
- ✅ Initializes database schema
- ✅ Indexes files successfully
- ✅ Reports statistics
- ✅ Handles errors gracefully

### FR4: Platform Knowledge Support ⏳ IN PROGRESS

**Status:** Designed but not implemented

**Requirements:**
- Support both `.h` header files AND `.json` knowledge files
- Parse platform `.h` files to extract:
  - Hardware constants (KNOB_1, FOOTSWITCH_1, etc.)
  - Method signatures
  - Initialization patterns
- Store platform knowledge separately from project knowledge
- Enable users to add new platforms

**Proposed Commands:**
```bash
# Add platform from header file
daisy-agent platform add hothouse --header path/to/hothouse.h

# Add platform from JSON
daisy-agent platform add terrarium --json path/to/terrarium.json

# List platforms
daisy-agent platform list
```

**Acceptance Criteria:**
- ⏳ Parse .h files for hardware constants
- ⏳ Parse .json platform definitions
- ⏳ Store in ~/daisy-agent/platforms/
- ⏳ Index platform knowledge into database
- ⏳ Include in compressed context when relevant

### FR5: Token Tracking ⏳ NOT STARTED

**Status:** Deferred to Phase 1

**Requirements:**
- Track cumulative token usage during session
- Alert at thresholds (60%, 40%, 20% remaining)
- Suggest handoff when approaching limit
- Conservative estimation (overestimate to be safe)

**Note:** Currently not possible to track web interface sessions. Would require:
- API integration, OR
- Manual token reporting from user, OR
- Browser extension (fragile)

### FR6: Mode Enforcement ⏳ NOT STARTED

**Status:** Deferred to Phase 1

**Requirements:**
- Validate user actions against current mode rules
- Block violations (e.g., auto-implementing in GUIDED mode)
- Provide clear feedback when rules violated
- Learn from user corrections

**Note:** Requires API integration or browser extension to intercept Claude's behavior.

---

## Non-Functional Requirements

### NFR1: Performance ✅ MET

**Requirements:**
- Context compression: <2 seconds ✅ (currently <100ms)
- Search: <100ms ✅ (currently <2ms)
- Handoff generation: <5 seconds ✅ (currently <1s)

### NFR2: Reliability ✅ MET

**Requirements:**
- Graceful error handling ✅
- No data loss ✅ (saves to file + clipboard)
- Works offline ✅ (after initial setup)

### NFR3: Usability ✅ MET

**Requirements:**
- Simple CLI interface ✅
- Clear help messages ✅
- Pretty output with tables ✅
- Minimal configuration ✅

### NFR4: Maintainability ✅ MET

**Requirements:**
- Pure Python ✅
- No compiled dependencies ✅
- Modular code structure ✅
- Standard project layout ✅

### NFR5: Installability ✅ MET

**Requirements:**
- Standard pip install ✅ (`pip install -e .`)
- Virtual environment support ✅
- Works with Homebrew Python ✅
- Clear installation docs ⏳ (to be written)

---

## Technical Architecture

### Project Structure

```
~/Desktop/+DEVELOPMENT+/DevTools/daisy-agent/
├── src/
│   └── daisy_agent/
│       ├── __init__.py
│       ├── cli.py              # Command-line interface
│       ├── compressor.py       # Context compression engine
│       ├── database.py         # SQLite knowledge base
│       └── handoff.py          # Handoff generation
├── platforms/                   # Platform knowledge templates
├── tests/                       # Unit tests (to be written)
├── venv/                        # Virtual environment
├── setup.py                     # Package configuration
├── requirements.txt             # Dependencies
└── README.md                    # Documentation

~/daisy-agent/                   # User data directory
├── knowledge.db                 # SQLite database
├── handoffs/                    # Generated handoff documents
│   └── FLUX_20251017_2008.md
└── platforms/                   # Platform knowledge (future)
```

### Dependencies

**Runtime (installed):**
- click==8.3.0 (CLI framework)
- PyYAML==6.0.3 (config files)
- rich==14.2.0 (terminal formatting)
- pyperclip==1.11.0 (clipboard operations)
- markdown-it-py==4.0.0 (rich dependency)
- Pygments==2.19.2 (rich dependency)
- mdurl==0.1.2 (rich dependency)

**Standard library (no install):**
- sqlite3 (database)
- json (parsing)
- hashlib (embeddings)
- pathlib (file handling)
- datetime (timestamps)

### Data Flow

```
User Command
    ↓
CLI (cli.py)
    ↓
Compressor (compressor.py) ←→ Database (database.py)
    ↓                              ↓
Handoff Generator              Knowledge Base
(handoff.py)                   (knowledge.db)
    ↓
Output: File + Clipboard
```

### Database Schema

```sql
CREATE TABLE chunks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    source TEXT NOT NULL,              -- Filename
    section TEXT NOT NULL,              -- JSON key or section name
    content TEXT NOT NULL,              -- Actual content
    tokens INTEGER NOT NULL,            -- Estimated token count
    embedding_hash TEXT NOT NULL,       -- MD5 of sorted keywords
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_source ON chunks(source);
CREATE INDEX idx_embedding ON chunks(embedding_hash);
```

# PRD Update: Template System Architecture
## Section to Add to daisy_agent_prd_v2.md

**Insert after "Technical Architecture" section**

---

## Template System Architecture

### Overview

The daisy-agent uses a three-template system to manage session transitions:

1. **session-end.json** - Instructions for Claude
2. **handoff.json** - For continuing existing projects  
3. **session-start.json** - For starting new projects

### Template Purposes

#### session-end.json
**Location:** `~/daisy-agent/templates/core/session-end.json`  
**Type:** Schema/Instructions  
**Used by:** Claude (AI assistant)

**Purpose:**  
Defines the JSON schema that Claude should follow when generating end-of-session output. This is NOT a template for generation, but rather instructions FOR the AI.

**Contains:**
- Required JSON schema fields
- Validation rules
- Scope detection guidelines (project vs global)
- Example output format
- Instructions for what to include

**Workflow:**
```
User says "create handoff" 
  → Claude reads session-end.json schema
  → Claude generates JSON following that schema
  → User copies JSON to clipboard
```

#### handoff.json
**Location:** `~/daisy-agent/templates/core/handoff.json`  
**Type:** Template  
**Used by:** `build-handoff` command

**Purpose:**  
Template for generating session-start documents for CONTINUING existing projects. Contains rich context from ongoing work.

**Contains:**
- Previous session summary
- Artifacts from current session
- Lessons learned (current + relevant DB history)
- Project patterns and context
- Next steps and suggested focus
- Compressed knowledge base excerpts
- Token reporting format
- Behavioral rules

**Workflow:**
```
User runs: daisy-agent build-handoff --project <name>
  → Reads session-end JSON from clipboard
  → Saves lessons to database
  → Loads handoff.json template
  → Substitutes variables with:
    * Session data from JSON
    * Recent artifacts from database
    * Relevant lessons from database (project + global)
    * Compressed knowledge excerpts
  → Generates populated session-start JSON
  → Saves to ~/daisy-agent/handoffs/
  → Copies to clipboard for next session
```

**Key Characteristic:** RICH context from ongoing project work

#### session-start.json
**Location:** `~/daisy-agent/templates/core/session-start.json`  
**Type:** Template  
**Used by:** Manual workflow OR future `project init` command

**Purpose:**  
Template for generating session-start documents for NEW projects that have no previous session history.

**Contains:**
- Project metadata (name, platform, mode)
- Basic startup checklist
- Platform-specific initialization info
- Token reporting format
- Behavioral rules
- NO previous session data (doesn't exist yet)
- NO artifacts or lessons (starting fresh)

**Workflow:**
```
Starting new project:
  → User manually creates initial context
  → OR: Future command: daisy-agent project init <name>
  → Uses session-start.json template
  → Minimal variable substitution
  → Generates initialization document
```

**Key Characteristic:** MINIMAL context for fresh starts

### Template Versions

| Template | Current Version | Status |
|----------|----------------|--------|
| session-end.json | 1.0 | Stable |
| handoff.json | 1.0 | Needs token reporting update |
| session-start.json | 1.1 | Updated with token reporting |

**Issue:** handoff.json v1.0 missing token_reporting_format section that exists in session-start.json v1.1

**Resolution Required:** Add token_reporting_format to handoff.json

### Session Workflow

#### Continuing Project (Typical)
```
SESSION N (Current)
  User: "create handoff"
  Claude: Generates JSON per session-end.json schema
  User: Copies JSON
  
  $ daisy-agent build-handoff --project myproject
    → Uses handoff.json template
    → Populates with session data + DB context
    → Generates rich session-start JSON
  
SESSION N+1 (Next)
  User: Pastes session-start JSON into Claude
  Claude: Loads context, ready to continue
```

#### New Project
```
NEW PROJECT START
  User: Creates initial context
  OR: $ daisy-agent project init myproject (future)
    → Uses session-start.json template
    → Minimal context
    → Generates initialization JSON
  
  User: Pastes initialization JSON into Claude
  Claude: Ready to start fresh project
```

### Design Principles

1. **Separation of Concerns:**
   - session-end.json = Instructions (what to generate)
   - handoff.json = Rich continuation (ongoing work)
   - session-start.json = Minimal initialization (new work)

2. **No Template Confusion:**
   - Each template serves distinct purpose
   - Names reflect usage, not output
   - Never merge without understanding purpose

3. **Variable Substitution:**
   - Templates contain {variable} placeholders
   - template_loader.py handles substitution
   - Preserves types (lists, dicts, strings)

4. **Token Budget Management:**
   - Templates include token reporting format
   - Handoffs target 5-8K tokens
   - Compression applied via template_loader

### File Locations

```
~/daisy-agent/
└── templates/
    ├── core/
    │   ├── session-end.json    (Claude instructions)
    │   ├── handoff.json         (Continue projects)
    │   └── session-start.json   (New projects)
    ├── platforms/               (Platform-specific)
    ├── domains/                 (Domain-specific)
    └── projects/                (Project-specific)
```

### Implementation Status

| Component | Status | Notes |
|-----------|--------|-------|
| session-end.json | ✅ Complete | Stable schema |
| handoff.json | ⚠️ Needs update | Missing token reporting |
| session-start.json | ✅ Complete | v1.1 with token reporting |
| template_loader.py | ✅ Complete | Variable substitution working |
| build-handoff command | ✅ Complete | Uses handoff.json |

### Critical Notes

⚠️ **DO NOT LOSE ARCHITECTURAL DISTINCTION**
- These templates serve different purposes
- Loss of distinction = loss of functionality
- Document changes clearly in changelog

⚠️ **VERSION MANAGEMENT**
- Track template versions separately
- Update all templates when adding common features
- Test handoff generation after template changes

---

## Version History

| Version | Date | Section | Change |
|---------|------|---------|--------|
| 2.1 | Oct 26, 2025 | NEW | Added Template System Architecture section |

---

## Implementation Status

### Phase 0: Handoff Assistant ✅ COMPLETE

**Timeline:** October 17, 2025 (1 session)  
**Goal:** Prove context compression works and enable basic handoffs

**Completed:**
- ✅ Project structure created
- ✅ Virtual environment setup
- ✅ Dependencies installed
- ✅ Database module implemented
- ✅ Compressor module implemented
- ✅ Handoff generator implemented
- ✅ CLI with 4 commands (init, index, compress, handoff, stats)
- ✅ Tested with real project files
- ✅ Package installed as `daisy-agent` command

**Deliverables:**
- Working CLI tool
- 57% compression on test data (will improve with more docs)
- Handoff documents generated and saved
- Code in version control ready state

### Phase 1: Enhanced Compression & Platform Support ⏳ PLANNED

**Timeline:** TBD (2-3 sessions estimated)  
**Goal:** Better compression, platform abstraction, more knowledge indexed

**Tasks:**
- [ ] Implement relevance scoring (Jaccard similarity)
- [ ] Add .h file parser for platform knowledge
- [ ] Support both .h and .json platform files
- [ ] Index all project knowledge files
- [ ] Improve compression ratio to 70%+
- [ ] Add query expansion for better search
- [ ] Write unit tests

### Phase 2: API Integration ⏳ PLANNED

**Timeline:** TBD (1-2 weeks estimated)  
**Goal:** Enable true "unlimited context" with Claude API

**Tasks:**
- [ ] Add Claude API client
- [ ] Implement automatic token tracking
- [ ] Build session continuity manager
- [ ] Add mode enforcement
- [ ] Automatic handoff when approaching token limit
- [ ] Multi-session task orchestration

**Note:** Requires user to get Claude API key and accept API costs (~$20-25/month estimated)

### Phase 3: Advanced Features ⏳ FUTURE

**Timeline:** TBD  
**Goal:** Pattern learning, MCP integration, community features

**Tasks:**
- [ ] Learn from user corrections
- [ ] Build pattern database
- [ ] MCP server implementation
- [ ] Web dashboard (optional)
- [ ] Community sharing of platform knowledge
- [ ] Plugin system for extensibility

---

## Success Metrics

### Phase 0 Metrics ✅ ACHIEVED

| Metric | Target | Actual | Status |
|--------|--------|--------|--------|
| Tool installable | Yes | Yes | ✅ |
| Commands working | 4+ | 5 | ✅ |
| Files indexed | 3+ | 3 | ✅ |
| Compression ratio | 50%+ | 57.6% | ✅ |
| Handoff generation | Working | Working | ✅ |
| Time to setup | <30 min | ~25 min | ✅ |

### Phase 1 Metrics (Target)

| Metric | Target |
|--------|--------|
| Compression ratio | 70%+ |
| Knowledge base | 10+ files |
| Platforms supported | 3+ (Hothouse, Funbox, Terrarium) |
| Search relevance | >80% accurate |
| Platform detection | Automatic |

### Phase 2 Metrics (Target)

| Metric | Target |
|--------|--------|
| Token tracking | Real-time |
| Automatic handoffs | 100% |
| Mode violations | 0 |
| API latency | <500ms |
| Session continuity | Seamless |

---

## Open Questions & Decisions Needed

### Immediate (Phase 1)

1. **Platform .h parser:** Which library? (regex, pycparser, clang-python?)
2. **Relevance algorithm:** Jaccard vs. cosine similarity vs. BM25?
3. **Query expansion:** Manual synonyms or automatic?
4. **Testing strategy:** Unit tests first or integration tests?

### Medium-term (Phase 2)

5. **API vs. web interface:** Force API migration or support both?
6. **Token tracking:** How to track web sessions without API?
7. **Mode enforcement:** Browser extension or API-only?

### Long-term (Phase 3)

8. **MCP integration:** When and how?
9. **Community platform:** GitHub-based or custom?
10. **Monetization:** Free/open-source vs. paid features?

---

## Risks & Mitigation

### Technical Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Hash embeddings insufficient | Medium | Medium | Can upgrade to sentence-transformers if needed |
| .h parsing too complex | Low | Medium | Start with JSON, add .h parsing incrementally |
| SQLite performance issues | Low | Low | Adequate for <10K chunks, can optimize if needed |
| API costs too high | Medium | High | Provide clear estimates, keep web interface option |

### Adoption Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Too complex to use | Low | High | Focus on simplicity, good docs, examples |
| Doesn't fit workflow | Low | High | Built with user, iterative feedback |
| Learning curve too steep | Medium | Medium | Video tutorials, interactive setup |

### Business Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| User switches to API, saves money | High | Positive! | This is a feature, not a risk |
| Tool abandonment | Low | Medium | Solve real problems, deliver value early |
| Community doesn't form | Medium | Low | Start with single-user tool, community is bonus |

---

## Dependencies & Constraints

### Hard Dependencies

- Python 3.10+ (user has 3.13.7 ✅)
- Homebrew package manager (macOS) ✅
- Terminal/command-line access ✅
- Internet for pip install (one-time) ✅

### Soft Dependencies (Future)

- Claude API key (for Phase 2)
- Git (for version control, optional)
- Web browser (for web interface option)

### Constraints

- Must work with existing project structure (non-invasive)
- Must handle .h files AND .json platform definitions
- Must work offline after setup (except API calls)
- Must be fast enough for daily use (<2s operations)

---

## Appendix A: Commands Reference

### Current Commands (Phase 0)

```bash
# Initialize agent
daisy-agent init

# Index knowledge file
daisy-agent index <filepath>

# Show knowledge base statistics  
daisy-agent stats

# Test compression
daisy-agent compress "query text"

# Generate handoff (interactive)
daisy-agent handoff

# Generate handoff (CLI)
daisy-agent handoff --project NAME --current "description" --working-file file.cpp

# Show help
daisy-agent --help
daisy-agent <command> --help
```

### Planned Commands (Phase 1+)

```bash
# Platform management
daisy-agent platform add <name> --header <file.h>
daisy-agent platform add <name> --json <file.json>
daisy-agent platform list

# Project management  
daisy-agent project init
daisy-agent project status

# Session management (Phase 2)
daisy-agent session start
daisy-agent session status
daisy-agent session end
```

---

## Appendix B: File Formats

### Handoff Document Format

```markdown
# Session Handoff: PROJECT - YYYY-MM-DD HH:MM

## SESSION INFO

**Mode:** HANDOFF
**Project:** project-name
**Current Work:** description
**Working Code:** filename.cpp (optional)

## COMPRESSED CONTEXT (X,XXX tokens)

[Compressed knowledge chunks]

## NEXT SESSION

When starting the next session, use GUIDED mode for continued development.

---
*Generated by daisy-agent v0.1.0*
```

### Platform Knowledge JSON Format (Proposed)

```json
{
  "platform": "hothouse",
  "manufacturer": "Cleveland Music Co.",
  "hardware": {
    "knobs": ["KNOB_1", "KNOB_2", "KNOB_3", "KNOB_4", "KNOB_5", "KNOB_6"],
    "toggles": ["TOGGLESWITCH_1", "TOGGLESWITCH_2", "TOGGLESWITCH_3"],
    "footswitches": ["FOOTSWITCH_1", "FOOTSWITCH_2"],
    "leds": ["LED_1", "LED_2"]
  },
  "api": {
    "init": "hw.Init(true);",
    "read_knob": "hw.GetKnobValue(Hothouse::KNOB_1)",
    "read_toggle": "hw.GetToggleswitchPosition(Hothouse::TOGGLESWITCH_1)"
  },
  "patterns": {
    "includes": ["daisy_seed.h", "daisysp.h", "hothouse.h"],
    "namespace": "using namespace clevelandmusicco;"
  }
}
```

---

## Appendix C: Development Environment

### User's Setup

- **OS:** macOS 15.6.1
- **Python:** 3.13.7 (Homebrew)
- **Location:** /opt/homebrew/bin/python3
- **Package Manager:** Homebrew + pip
- **Terminal:** Standard macOS Terminal
- **Editor:** (not specified)

### Project Location

- **Development:** `~/Desktop/+DEVELOPMENT+/DevTools/daisy-agent/`
- **User Data:** `~/daisy-agent/`
- **Virtual Env:** `~/Desktop/+DEVELOPMENT+/DevTools/daisy-agent/venv/`

### Installation Process

```bash
cd ~/Desktop/+DEVELOPMENT+/DevTools/daisy-agent/
python3 -m venv venv
source venv/bin/activate
pip install click pyyaml rich pyperclip
pip install -e .
```

### Daily Usage

```bash
# Activate virtual environment when developing
cd ~/Desktop/+DEVELOPMENT+/DevTools/daisy-agent/
source venv/bin/activate

# Use daisy-agent command anywhere
daisy-agent <command>

# Deactivate when done developing
deactivate
```

---

## Document History

**v1.0 (Oct 16, 2025):**
- Initial PRD created
- Full architecture designed
- 4-phase roadmap established

**v2.0 (Oct 17, 2025):**
- Phase 0 implemented and tested
- Renamed to daisy-agent (platform-agnostic)
- Mode system updated (GUIDED/HANDOFF)
- Virtual environment requirement added
- Real performance metrics added
- Updated technical architecture
- Added installation appendix

---

**Next Review:** After Phase 1 completion  
**Status:** Living document - update as implementation progresses
