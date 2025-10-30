#!/bin/bash

# checkpoint.sh - Create git checkpoint for private projects
# Usage: ./tools/checkpoint.sh <project> <message> [--build] [--tag version]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse arguments
PROJECT=$1
MESSAGE=$2
BUILD=false
TAG=""

if [ -z "$PROJECT" ] || [ -z "$MESSAGE" ]; then
    echo -e "${RED}Usage: ./tools/checkpoint.sh <project> <message> [--build] [--tag version]${NC}"
    echo ""
    echo "Projects: flux, tremodulay, ambien, buzzbox"
    echo ""
    echo "Examples:"
    echo "  ./tools/checkpoint.sh flux \"working delay sync\""
    echo "  ./tools/checkpoint.sh flux \"v1.2 release\" --build --tag v1.2"
    exit 1
fi

# Parse optional flags
shift 2
while [[ $# -gt 0 ]]; do
    case $1 in
        --build)
            BUILD=true
            shift
            ;;
        --tag)
            TAG="$2"
            shift 2
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Map project names to directories
case $PROJECT in
    flux)
        PROJECT_DIR="original-hothouse-projects/flux-hothouse"
        PROJECT_NAME="FLUX"
        ;;
    tremodulay)
        PROJECT_DIR="original-hothouse-projects/tremodulay-hothouse"
        PROJECT_NAME="Tremodulay"
        ;;
    ambien)
        PROJECT_DIR="original-hothouse-projects/ambien-hothouse"
        PROJECT_NAME="Ambien"
        ;;
    buzzbox)
        PROJECT_DIR="original-hothouse-projects/buzzbox-hothouse"
        PROJECT_NAME="BuzzBox"
        ;;
    *)
        echo -e "${RED}Unknown project: $PROJECT${NC}"
        echo "Valid projects: flux, tremodulay, ambien, buzzbox"
        exit 1
        ;;
esac

# Check if project directory exists
if [ ! -d "$PROJECT_DIR" ]; then
    echo -e "${RED}Project directory not found: $PROJECT_DIR${NC}"
    exit 1
fi

echo -e "${BLUE}=== Checkpoint: $PROJECT_NAME ===${NC}"
echo -e "Message: ${YELLOW}$MESSAGE${NC}"
echo ""

# Build if requested
if [ "$BUILD" = true ]; then
    echo -e "${BLUE}Building $PROJECT_NAME...${NC}"
    cd "$PROJECT_DIR"
    
    # Clean and rebuild
    make clean > /dev/null 2>&1 || true
    if make -j; then
        echo -e "${GREEN}✓ Build successful${NC}"
        
        # Copy binary to binary/ directory if it exists
        if [ -f "build/${PROJECT_NAME}.bin" ]; then
            mkdir -p binary
            TIMESTAMP=$(date +%Y%m%d_%H%M%S)
            cp "build/${PROJECT_NAME}.bin" "binary/${PROJECT_NAME}_${TIMESTAMP}.bin"
            echo -e "${GREEN}✓ Binary saved: binary/${PROJECT_NAME}_${TIMESTAMP}.bin${NC}"
        fi
    else
        echo -e "${RED}✗ Build failed${NC}"
        cd ../..
        exit 1
    fi
    cd ../..
    echo ""
fi

# Check for changes
if git diff --quiet "$PROJECT_DIR" && git diff --cached --quiet "$PROJECT_DIR"; then
    echo -e "${YELLOW}No changes detected in $PROJECT_DIR${NC}"
    exit 0
fi

# Show what's changed
echo -e "${BLUE}Changes to be committed:${NC}"
git diff --stat "$PROJECT_DIR"
echo ""

# Stage changes
echo -e "${BLUE}Staging changes...${NC}"
git add "$PROJECT_DIR"

# Create commit
COMMIT_MSG="$PROJECT_NAME: $MESSAGE"
echo -e "${BLUE}Creating commit...${NC}"
git commit -m "$COMMIT_MSG"

COMMIT_HASH=$(git rev-parse --short HEAD)
echo -e "${GREEN}✓ Checkpoint created: $COMMIT_HASH${NC}"
echo ""

# Create tag if requested
if [ -n "$TAG" ]; then
    TAG_NAME="${PROJECT,,}-$TAG"  # Lowercase project + tag
    echo -e "${BLUE}Creating tag: $TAG_NAME${NC}"
    git tag -a "$TAG_NAME" -m "$PROJECT_NAME $TAG: $MESSAGE"
    echo -e "${GREEN}✓ Tag created: $TAG_NAME${NC}"
    echo ""
fi

# Show recent history
echo -e "${BLUE}Recent commits for $PROJECT_NAME:${NC}"
git log --oneline --decorate -5 -- "$PROJECT_DIR"
echo ""

# Show project status
echo -e "${GREEN}=== Checkpoint Complete ===${NC}"
echo -e "Project: ${YELLOW}$PROJECT_NAME${NC}"
echo -e "Commit: ${YELLOW}$COMMIT_HASH${NC}"
if [ -n "$TAG" ]; then
    echo -e "Tag: ${YELLOW}$TAG_NAME${NC}"
fi
echo ""

# Optional: Show tags for this project
TAGS=$(git tag -l "${PROJECT,,}-*" 2>/dev/null)
if [ -n "$TAGS" ]; then
    echo -e "${BLUE}Tags for $PROJECT_NAME:${NC}"
    echo "$TAGS"
fi
