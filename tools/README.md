# Development Tools

This directory contains tools for managing the daisy-seed-pedals repository.

## Checkpoint Scripts

Git-based version control tools for creating and viewing project checkpoints.

### checkpoint.sh

Create git checkpoints for private projects with optional builds and tags.

**Usage:**
```bash
# From repository root
./tools/checkpoint.sh <project> <message> [--build] [--tag version]
```

**Projects:** flux, tremodulay, ambien, buzzbox

**Examples:**
```bash
# Simple checkpoint
./tools/checkpoint.sh flux "working delay sync implementation"

# Checkpoint with build
./tools/checkpoint.sh flux "added sample and hold mode" --build

# Major version with tag
./tools/checkpoint.sh flux "v2.0 release" --build --tag v2.0
```

**Features:**
- Creates git commit for project changes
- Optional build flag compiles and saves timestamped binary
- Optional tag flag creates git tag for version milestones
- Shows recent commit history
- Color-coded output for status

### list-checkpoints.sh

View checkpoint history for projects.

**Usage:**
```bash
# From repository root
./tools/list-checkpoints.sh [project] [--tags]
```

**Examples:**
```bash
# View FLUX checkpoints
./tools/list-checkpoints.sh flux

# View FLUX checkpoints with tags
./tools/list-checkpoints.sh flux --tags

# View all private project checkpoints
./tools/list-checkpoints.sh

# View all project tags
./tools/list-checkpoints.sh --tags
```

**Features:**
- Shows last 20 commits for specified project
- Can display git tags for version tracking
- Color-coded output

## Git-Based Workflow

These scripts replace the old manual snapshot workflow. Instead of copying entire directories, use git commits and tags for version control.

**Migration from old workflow:**
- ❌ Old: `cp -r flux flux_snapshot_20251029`
- ✅ New: `./tools/checkpoint.sh flux "checkpoint description"`

**Advantages:**
- Full git history and diffs
- No wasted disk space
- Easy rollback to any commit
- Professional version control
- Tag stable versions

## daisy-agent

CLI tool for managing development sessions, handoffs, and knowledge base.

**Installation:**
```bash
cd tools/daisy-agent
python -m venv venv
source venv/bin/activate  # or venv\Scripts\activate on Windows
pip install -e .
```

**Usage:**
```bash
# View help
daisy-agent --help

# Common commands
daisy-agent note --project FLUX --category lesson --text "Important lesson"
daisy-agent build-handoff --project FLUX
daisy-agent session-status
```

**Features:**
- Session handoff generation with compression
- Knowledge base for lessons and patterns
- Artifact tracking across sessions
- Token-efficient context management

See `daisy-agent/docs/` for detailed documentation.

## Repository Structure

```
daisy-seed-pedals/
├── tools/
│   ├── checkpoint.sh          # Git checkpoint creation
│   ├── list-checkpoints.sh    # View checkpoint history
│   └── daisy-agent/          # Session management tool
├── original-hothouse-projects/
│   ├── flux-hothouse/        # Private (local only)
│   ├── tremodulay-hothouse/  # Private (local only)
│   ├── ambien-hothouse/      # Private (local only)
│   └── buzzbox-hothouse/     # Public (pushed to GitHub)
└── funbox-to-hothouse-ports/
    ├── venus-hothouse/       # Public (pushed to GitHub)
    ├── mars-hothouse/        # Public (pushed to GitHub)
    └── earth-hothouse/       # Public (pushed to GitHub)
```

**Private Projects:** Version controlled locally, never pushed to GitHub (listed in `.gitignore`)
**Public Projects:** Committed and pushed to public repository
