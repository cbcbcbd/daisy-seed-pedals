#!/usr/bin/env python3
"""
Test the note command functionality.
"""

from rich.console import Console
from rich.table import Table

console = Console()

def test_note():
    """Test the note command with examples."""
    
    console.print("\n[cyan]🧪 Testing daisy-agent note command[/cyan]\n")
    
    # Create test commands table
    table = Table(title="Test Note Commands", show_header=True)
    table.add_column("Category", style="cyan")
    table.add_column("Command", style="green")
    
    table.add_row(
        "💡 lesson",
        "daisy-agent note --project FLUX \\\n  --category lesson \\\n  --text 'Reverse delay crossfading prevents clicks'"
    )
    
    table.add_row(
        "🔧 pattern",
        "daisy-agent note --project FLUX \\\n  --category pattern \\\n  --text 'OnePole filter for smooth parameter changes'"
    )
    
    table.add_row(
        "⚠️  gotcha",
        "daisy-agent note --project FLUX \\\n  --category gotcha \\\n  --text 'Delay feedback sounds different on hardware'"
    )
    
    table.add_row(
        "✨ tip",
        "daisy-agent note --project FLUX \\\n  --category tip \\\n  --text 'Use fonepole coefficient 0.0002 for delay time smoothing'"
    )
    
    table.add_row(
        "🎯 decision",
        "daisy-agent note --project FLUX \\\n  --category decision \\\n  --text 'Using Neptune delayline library (MIT licensed)'"
    )
    
    console.print(table)
    
    console.print("\n[yellow]💡 Auto-detection (no --category flag):[/yellow]")
    console.print("  daisy-agent note --project FLUX 'Always test on hardware'")
    console.print("  [dim]→ Auto-detects as 'lesson' from keyword 'always'[/dim]\n")
    
    console.print("  daisy-agent note --project FLUX 'Hardware delay sounds different'")
    console.print("  [dim]→ Auto-detects as 'gotcha' from keyword 'hardware'[/dim]\n")
    
    console.print("[green]✅ Try running any of the commands above![/green]\n")
    
    console.print("[cyan]📊 After adding notes, verify with:[/cyan]")
    console.print("  python tests/verify_notes.py\n")


if __name__ == "__main__":
    test_note()
