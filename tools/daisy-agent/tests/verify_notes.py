#!/usr/bin/env python3
"""
Verify that notes were saved correctly.
"""

from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).parent.parent / "src"))

from daisy_agent.database import KnowledgeDB
from rich.console import Console
from rich.table import Table
from rich.panel import Panel

console = Console()

def verify_notes():
    """Check what notes are in the database."""
    
    db_path = Path.home() / "daisy-agent" / "knowledge.db"
    
    if not db_path.exists():
        console.print("[red]✗[/red] Database not found. Run 'daisy-agent init' first.")
        return
    
    db = KnowledgeDB(db_path)
    
    console.print("\n[cyan]📝 Checking saved notes...[/cyan]\n")
    
    # Get all projects
    projects = db.list_projects()
    
    if not projects:
        console.print("[yellow]No projects found yet.[/yellow]")
        db.close()
        return
    
    # Category emojis
    category_emoji = {
        'lesson': '💡',
        'pattern': '🔧',
        'gotcha': '⚠️',
        'tip': '✨',
        'decision': '🎯'
    }
    
    # Show notes for each project
    for project_info in projects:
        project = project_info['name']
        notes = db.get_notes(project)
        
        if notes:
            console.print(f"[green]Project:[/green] {project}\n")
            
            # Get notes by category
            notes_by_cat = db.get_notes_by_category(project)
            
            # Show summary
            summary_parts = []
            for cat, cat_notes in notes_by_cat.items():
                if cat_notes:
                    emoji = category_emoji.get(cat, '📝')
                    summary_parts.append(f"{emoji} {cat}: {len(cat_notes)}")
            
            if summary_parts:
                console.print(f"[cyan]Summary:[/cyan] {' | '.join(summary_parts)}\n")
            
            # Show detailed table
            table = Table(show_header=True, header_style="bold cyan")
            table.add_column("ID", style="dim", width=4)
            table.add_column("Category", width=10)
            table.add_column("Note", width=60)
            table.add_column("Created", width=16)
            
            for note in notes:
                emoji = category_emoji.get(note['category'], '📝')
                cat_display = f"{emoji} {note['category']}"
                
                # Truncate long notes
                note_text = note['text']
                if len(note_text) > 60:
                    note_text = note_text[:57] + "..."
                
                # Format timestamp
                created = note['created_at'][:16] if note['created_at'] else "-"
                
                table.add_row(
                    str(note['id']),
                    cat_display,
                    note_text,
                    created
                )
            
            console.print(table)
            console.print()
    
    # Show stats
    stats = db.get_stats()
    console.print(f"[cyan]Total notes:[/cyan] {stats['notes']}")
    console.print(f"[cyan]Total projects with notes:[/cyan] {stats['note_projects']}")
    
    db.close()


if __name__ == "__main__":
    verify_notes()
