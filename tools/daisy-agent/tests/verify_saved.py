#!/usr/bin/env python3
"""
Verify that artifacts were saved correctly.
"""

from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).parent / "src"))

from daisy_agent.database import KnowledgeDB
from rich.console import Console
from rich.table import Table

console = Console()

def verify_save():
    """Check what artifacts are in the database."""
    
    db_path = Path.home() / "daisy-agent" / "knowledge.db"
    
    if not db_path.exists():
        console.print("[red]✗[/red] Database not found. Run 'daisy-agent init' first.")
        return
    
    db = KnowledgeDB(db_path)
    
    console.print("\n[cyan]📦 Checking saved artifacts...[/cyan]\n")
    
    # Get all artifacts
    projects = db.list_projects()
    
    if not projects:
        console.print("[yellow]No projects found yet.[/yellow]")
        console.print("Run: daisy-agent save --project FLUX --file test.cpp")
        db.close()
        return
    
    # Show artifacts for each project
    for project_info in projects:
        project = project_info['name']
        artifacts = db.get_artifacts(project)
        
        if artifacts:
            console.print(f"[green]Project:[/green] {project}")
            
            table = Table(show_header=True, header_style="bold cyan")
            table.add_column("ID", style="dim")
            table.add_column("Filename")
            table.add_column("Type")
            table.add_column("Size")
            table.add_column("Description")
            
            for art in artifacts:
                size_kb = art['size_bytes'] / 1024
                desc = art['description'] or "-"
                table.add_row(
                    str(art['id']),
                    art['filename'],
                    art['file_type'] or "-",
                    f"{size_kb:.1f} KB",
                    desc[:40] + "..." if len(desc) > 40 else desc
                )
            
            console.print(table)
            console.print()
            
            # Check filesystem
            for art in artifacts:
                fs_path = Path.home() / "daisy-agent" / "projects" / project / "artifacts" / art['filename']
                if fs_path.exists():
                    console.print(f"  [green]✓[/green] File exists: {fs_path}")
                else:
                    console.print(f"  [red]✗[/red] File not found: {fs_path}")
            
            console.print()
    
    # Show stats
    stats = db.get_stats()
    console.print(f"[cyan]Total artifacts:[/cyan] {stats['artifacts']}")
    console.print(f"[cyan]Total projects:[/cyan] {stats['projects']}")
    
    db.close()


if __name__ == "__main__":
    verify_save()
