#!/usr/bin/env python3
"""
Test the session-status command.
"""

from rich.console import Console
from rich.panel import Panel

console = Console()

def test_session_status():
    """Show how to use session-status command."""
    
    console.print("\n[cyan]🧪 Testing daisy-agent session-status command[/cyan]\n")
    
    console.print("[green]Basic usage:[/green]")
    console.print("  daisy-agent session-status --project FLUX\n")
    
    console.print("[green]With timestamp filter:[/green]")
    console.print("  daisy-agent session-status --project FLUX \\")
    console.print("    --since '2025-10-18 22:00'\n")
    
    console.print("[yellow]What you'll see:[/yellow]")
    
    expected = """📦 Session Status: FLUX

Artifacts (1):
  💾 flux_phase2.cpp (0.8 KB, code)
     Phase 2 with reverse delay

Notes (7):
  💡 Lessons: 2
     - Reverse delay crossfading prevents clicks
     - Always test delay feedback on hardware
  🔧 Patterns: 1
     - OnePole filter for smooth parameter changes
  ⚠️  Gotchas: 2
     - Delay feedback sounds different on hardware
     - Hardware delay sounds different in testing
  ✨ Tips: 1
     - Use fonepole coefficient 0.0002 for delay time smoothing
  🎯 Decisions: 1
     - Using Neptune delayline library (MIT licensed)

✓ Total items captured: 8

Ready to generate handoff with:
  daisy-agent handoff --project FLUX"""
    
    console.print(Panel(expected, title="Expected Output", border_style="green"))
    
    console.print("\n[cyan]💡 Use this during sessions to:[/cyan]")
    console.print("  • Review what you've captured")
    console.print("  • Verify notes and artifacts before handoff")
    console.print("  • Check if you missed anything important\n")
    
    console.print("[green]✅ Try running:[/green]")
    console.print("  daisy-agent session-status --project FLUX\n")


if __name__ == "__main__":
    test_session_status()
