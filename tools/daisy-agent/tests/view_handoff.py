#!/usr/bin/env python3
"""
View the most recent handoff document.
"""

from pathlib import Path
from rich.console import Console
from rich.markdown import Markdown

console = Console()

def view_latest_handoff(project: str = None):
    """View the most recent handoff document."""
    
    handoff_dir = Path.home() / "daisy-agent" / "handoffs"
    
    if not handoff_dir.exists():
        console.print("[red]✗[/red] No handoffs directory found.")
        return
    
    # Get all handoff files
    if project:
        handoffs = list(handoff_dir.glob(f"{project}_*.md"))
    else:
        handoffs = list(handoff_dir.glob("*.md"))
    
    if not handoffs:
        if project:
            console.print(f"[yellow]No handoffs found for project: {project}[/yellow]")
        else:
            console.print("[yellow]No handoffs found[/yellow]")
        return
    
    # Get the most recent
    latest = max(handoffs, key=lambda p: p.stat().st_mtime)
    
    console.print(f"\n[cyan]📄 Latest Handoff:[/cyan] {latest.name}\n")
    console.print(f"[dim]Path: {latest}[/dim]\n")
    
    # Read and display
    content = latest.read_text()
    
    # Display with markdown rendering
    console.print("=" * 80)
    md = Markdown(content)
    console.print(md)
    console.print("=" * 80)
    
    console.print(f"\n[green]✓[/green] Handoff is also in your clipboard!")
    console.print(f"[dim]Paste into Claude to start next session[/dim]\n")


if __name__ == "__main__":
    import sys
    
    project = sys.argv[1] if len(sys.argv) > 1 else None
    
    if project:
        console.print(f"[cyan]Viewing handoffs for project:[/cyan] {project}\n")
    
    view_latest_handoff(project)
