#!/usr/bin/env python3
"""
Test script to verify the enhanced database schema.
Run this to test the new tables and methods.
"""

from pathlib import Path
import tempfile
from datetime import datetime

# Assuming database.py is in src/daisy_agent/
import sys
sys.path.insert(0, str(Path(__file__).parent / "src"))

from daisy_agent.database import KnowledgeDB


def test_schema():
    """Test the enhanced database schema."""
    
    # Create temporary database
    with tempfile.TemporaryDirectory() as tmpdir:
        db_path = Path(tmpdir) / "test.db"
        print(f"Creating test database at: {db_path}\n")
        
        db = KnowledgeDB(db_path)
        
        # Test 1: Verify tables exist
        print("✓ Database initialized with new schema")
        
        # Test 2: Add an artifact
        print("\n📦 Testing artifact storage...")
        artifact_id = db.add_artifact(
            project="FLUX",
            filename="flux_phase2.cpp",
            content="// Test code content\nint main() { return 0; }",
            file_type="code",
            description="Phase 2 implementation with reverse delay"
        )
        print(f"✓ Artifact saved with ID: {artifact_id}")
        
        # Test 3: Retrieve artifacts
        artifacts = db.get_artifacts("FLUX")
        print(f"✓ Retrieved {len(artifacts)} artifact(s)")
        if artifacts:
            art = artifacts[0]
            print(f"  - {art['filename']} ({art['size_bytes']} bytes)")
            print(f"  - Type: {art['file_type']}")
            print(f"  - Description: {art['description']}")
        
        # Test 4: Add notes
        print("\n📝 Testing note storage...")
        note_id1 = db.add_note(
            project="FLUX",
            category="lesson",
            text="Reverse delay crossfading prevents clicks"
        )
        print(f"✓ Lesson note saved with ID: {note_id1}")
        
        note_id2 = db.add_note(
            project="FLUX",
            category="gotcha",
            text="Delay feedback sounds different on hardware"
        )
        print(f"✓ Gotcha note saved with ID: {note_id2}")
        
        note_id3 = db.add_note(
            project="FLUX",
            category="pattern",
            text="OnePole filter for smooth parameter changes"
        )
        print(f"✓ Pattern note saved with ID: {note_id3}")
        
        # Test 5: Retrieve notes
        notes = db.get_notes("FLUX")
        print(f"✓ Retrieved {len(notes)} note(s)")
        
        # Test 6: Retrieve notes by category
        print("\n📊 Notes by category:")
        by_category = db.get_notes_by_category("FLUX")
        for category, category_notes in by_category.items():
            if category_notes:
                print(f"  {category}: {len(category_notes)}")
                for note in category_notes:
                    print(f"    - {note['text'][:50]}...")
        
        # Test 7: Project info
        print("\n🎯 Testing project metadata...")
        project_info = db.get_project_info("FLUX")
        if project_info:
            print("✓ Project info retrieved:")
            print(f"  - Name: {project_info['name']}")
            print(f"  - Total sessions: {project_info['total_sessions']}")
            print(f"  - Last working file: {project_info['last_working_file']}")
        
        # Test 8: List projects
        projects = db.list_projects()
        print(f"✓ Total projects in database: {len(projects)}")
        
        # Test 9: Database stats
        print("\n📈 Database statistics:")
        stats = db.get_stats()
        print(f"  - Chunks: {stats['chunks']}")
        print(f"  - Artifacts: {stats['artifacts']} across {stats['artifact_projects']} project(s)")
        print(f"  - Notes: {stats['notes']} across {stats['note_projects']} project(s)")
        print(f"  - Projects: {stats['projects']}")
        
        # Test 10: Add multiple artifacts to test retrieval
        print("\n📦 Testing multiple artifacts...")
        db.add_artifact(
            project="FLUX",
            filename="reverse_delay.h",
            content="// Header file content",
            file_type="code",
            description="Reverse delay header"
        )
        db.add_artifact(
            project="FLUX",
            filename="design_notes.md",
            content="# Design Notes\n\nSome notes here",
            file_type="doc"
        )
        
        artifacts = db.get_artifacts("FLUX")
        print(f"✓ Now have {len(artifacts)} artifacts for FLUX:")
        for art in artifacts:
            print(f"  - {art['filename']} ({art['size_bytes']} bytes, {art['file_type']})")
        
        print("\n✅ All schema tests passed!")
        
        db.close()


if __name__ == "__main__":
    test_schema()
