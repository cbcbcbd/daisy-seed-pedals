#!/bin/bash

# list-checkpoints.sh - Show checkpoint history for projects
# Usage: ./list-checkpoints.sh [project] [--tags]

PROJECT=$1
SHOW_TAGS=false

if [ "$2" = "--tags" ] || [ "$1" = "--tags" ]; then
    SHOW_TAGS=true
fi

# Colors
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
NC='\033[0m'

if [ -n "$PROJECT" ] && [ "$PROJECT" != "--tags" ]; then
    # Map project name to directory
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
            echo "Unknown project: $PROJECT"
            exit 1
            ;;
    esac
    
    echo -e "${BLUE}=== Checkpoints for $PROJECT_NAME ===${NC}"
    git log --oneline --decorate -20 -- "$PROJECT_DIR"
    
    if [ "$SHOW_TAGS" = true ]; then
        echo ""
        echo -e "${BLUE}=== Tags for $PROJECT_NAME ===${NC}"
        git tag -l "${PROJECT,,}-*"
    fi
else
    # Show all private project commits
    echo -e "${BLUE}=== Recent Checkpoints (All Private Projects) ===${NC}"
    git log --oneline --decorate -20 -- original-hothouse-projects/
    
    if [ "$SHOW_TAGS" = true ]; then
        echo ""
        echo -e "${BLUE}=== All Project Tags ===${NC}"
        git tag -l "*-v*"
    fi
fi
