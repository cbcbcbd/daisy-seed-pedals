"""
Command-line interface for daisy-agent.
"""

import click
import pyperclip
from pathlib import Path
from rich.console import Console
from rich.table import Table
from rich.panel import Panel
from rich import box

from .database import KnowledgeDB
from .compressor import ContextCompressor
from .handoff import HandoffGenerator

console = Console()

# Agent home directory
AGENT_HOME = Path.home() / "daisy-agent"
DB_PATH = AGENT_HOME / "knowledge.db"


@click.group()
@click.version_option(version="0.2.0")
def cli():
    """Daisy Agent - Session orchestration for Daisy Seed development."""
    pass


@cli.command()
def init():
    """Initialize the daisy-agent environment."""
    try:
        # Create directories
        AGENT_HOME.mkdir(exist_ok=True)
        (AGENT_HOME / "handoffs").mkdir(exist_ok=True)
        (AGENT_HOME / "platforms").mkdir(exist_ok=True)
        (AGENT_HOME / "projects").mkdir(exist_ok=True)
        
        # Initialize database
        db = KnowledgeDB(DB_PATH)
        db.close()
        
        console.print("[green]✓[/green] Agent environment initialized")
        console.print(f"[dim]Database: {DB_PATH}[/dim]")
        console.print(f"[dim]Handoffs: {AGENT_HOME / 'handoffs'}[/dim]")
        console.print(f"[dim]Projects: {AGENT_HOME / 'projects'}[/dim]")
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
@click.argument('filepath', type=click.Path(exists=True))
def index(filepath):
    """Index a knowledge file into the database."""
    try:
        db = KnowledgeDB(DB_PATH)
        compressor = ContextCompressor(db)
        
        path = Path(filepath)
        console.print(f"Indexing [cyan]{path.name}[/cyan]...")
        
        chunks_added = compressor.index_file(path)
        
        console.print(f"[green]✓[/green] Indexed {chunks_added} chunks from {path.name}")
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
def stats():
    """Show knowledge base statistics."""
    try:
        db = KnowledgeDB(DB_PATH)
        stats = db.get_stats()
        
        # Create stats table
        table = Table(title="Knowledge Base Statistics", box=box.ROUNDED)
        table.add_column("Metric", style="cyan")
        table.add_column("Count", style="green", justify="right")
        
        table.add_row("Knowledge Chunks", str(stats['chunks']))
        table.add_row("Total Tokens", f"{stats['total_tokens']:,}")
        table.add_row("Source Files", str(stats['sources']))
        table.add_row("", "")  # Separator
        table.add_row("Artifacts", str(stats['artifacts']))
        table.add_row("Artifact Projects", str(stats['artifact_projects']))
        table.add_row("Notes", str(stats['notes']))
        table.add_row("Note Projects", str(stats['note_projects']))
        table.add_row("", "")  # Separator
        table.add_row("Total Projects", str(stats['projects']))
        
        console.print(table)
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
@click.argument('query')
@click.option('--tokens', default=2000, help='Target token budget')
def compress(query, tokens):
    """Test context compression with a query."""
    try:
        db = KnowledgeDB(DB_PATH)
        compressor = ContextCompressor(db)
        
        console.print(f"Query: [cyan]{query}[/cyan]")
        console.print(f"Target: [yellow]{tokens}[/yellow] tokens\n")
        
        compressed = compressor.compress_context(query, max_tokens=tokens)
        stats = compressor.get_compression_stats(query, max_tokens=tokens)
        
        console.print("[green]Compressed Context:[/green]")
        console.print(compressed)
        
        console.print(f"\n[dim]Compressed: {stats['compressed_tokens']} tokens[/dim]")
        console.print(f"[dim]Original: {stats['full_tokens']} tokens[/dim]")
        console.print(f"[dim]Compression: {stats['compression_pct']:.1f}%[/dim]")
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
@click.option('--project', prompt='Project name', help='Project name')
@click.option('--current', prompt='Current work', help='What you are working on')
@click.option('--working-file', default='', help='Current working file (optional)')
def handoff(project, current, working_file):
    """Generate a session handoff document."""
    try:
        db = KnowledgeDB(DB_PATH)
        generator = HandoffGenerator(db)
        
        console.print(f"\nGenerating handoff for [cyan]{project}[/cyan]...")
        
        # Generate handoff
        handoff_path = generator.generate_handoff(
            project=project,
            current_work=current,
            working_file=working_file or None
        )
        
        console.print(f"[green]✓[/green] Handoff saved: [cyan]{handoff_path}[/cyan]")
        console.print(f"[green]✓[/green] Copied to clipboard")
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
@click.option('--project', required=True, help='Project name')
@click.option('--since', help='Show items since timestamp (YYYY-MM-DD HH:MM)')
def session_status(project, since):
    """Show current session status (artifacts and notes captured)."""
    try:
        db = KnowledgeDB(DB_PATH)
        
        # Parse since timestamp if provided
        since_dt = None
        if since:
            from datetime import datetime
            try:
                since_dt = datetime.strptime(since, "%Y-%m-%d %H:%M")
            except ValueError:
                console.print("[red]✗[/red] Invalid timestamp format. Use: YYYY-MM-DD HH:MM")
                raise click.Abort()
        
        console.print(f"\n[cyan]📦 Session Status:[/cyan] [bold]{project}[/bold]\n")
        
        # Get project info
        project_info = db.get_project_info(project)
        if project_info:
            console.print(f"[dim]Last session: {project_info['last_session'] or 'Never'}[/dim]")
            if project_info['last_working_file']:
                console.print(f"[dim]Last working file: {project_info['last_working_file']}[/dim]")
            console.print()
        
        # Get artifacts
        artifacts = db.get_artifacts(project, since=since_dt)
        
        if artifacts:
            console.print(f"[green]Artifacts ({len(artifacts)}):[/green]")
            for art in artifacts:
                size_kb = art['size_bytes'] / 1024
                file_type = art['file_type'] or 'unknown'
                icon = "📄" if file_type == 'doc' else "⚙️" if file_type == 'config' else "💾"
                console.print(f"  {icon} [cyan]{art['filename']}[/cyan] ({size_kb:.1f} KB, {file_type})")
                if art['description']:
                    console.print(f"     [dim]{art['description']}[/dim]")
            console.print()
        else:
            console.print("[yellow]No artifacts captured yet[/yellow]\n")
        
        # Get notes
        notes = db.get_notes(project, since=since_dt)
        notes_by_cat = db.get_notes_by_category(project)
        
        # Filter by since if provided
        if since_dt and notes_by_cat:
            filtered_by_cat = {cat: [] for cat in notes_by_cat.keys()}
            for note in notes:
                filtered_by_cat[note['category']].append(note)
            notes_by_cat = filtered_by_cat
        
        if notes:
            category_emoji = {
                'lesson': '💡',
                'pattern': '🔧',
                'gotcha': '⚠️',
                'tip': '✨',
                'decision': '🎯'
            }
            
            console.print(f"[green]Notes ({len(notes)}):[/green]")
            for category, cat_notes in notes_by_cat.items():
                if cat_notes:
                    emoji = category_emoji.get(category, '📝')
                    console.print(f"  {emoji} [cyan]{category.capitalize()}s:[/cyan] {len(cat_notes)}")
                    
                    # Show first 3 notes in each category
                    for note in cat_notes[:3]:
                        note_text = note['text']
                        if len(note_text) > 60:
                            note_text = note_text[:57] + "..."
                        # Show scope if available
                        scope_icon = "🌍" if note.get('scope') == 'global' else "📦"
                        console.print(f"     {scope_icon} [dim]{note_text}[/dim]")
                    
                    if len(cat_notes) > 3:
                        console.print(f"     [dim]... and {len(cat_notes) - 3} more[/dim]")
            console.print()
        else:
            console.print("[yellow]No notes captured yet[/yellow]\n")
        
        # Summary
        total_items = len(artifacts) + len(notes)
        if total_items > 0:
            console.print(f"[green]✓[/green] Total items captured: {total_items}")
            console.print("\n[dim]Ready to generate handoff with:[/dim]")
            console.print(f"[dim]  daisy-agent handoff --project {project}[/dim]\n")
        else:
            console.print("[yellow]💡 Start capturing with:[/yellow]")
            console.print(f"[dim]  daisy-agent save --project {project} --file <filename>[/dim]")
            console.print(f"[dim]  daisy-agent note --project {project} 'your note'[/dim]\n")
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()


@cli.command()
@click.option('--project', required=True, help='Project name')
@click.option('--file', 'filename', required=True, help='Filename to save')
@click.option('--type', 'file_type', 
              type=click.Choice(['code', 'doc', 'config', 'other']),
              help='File type (auto-detected if not specified)')
@click.option('--description', help='Optional description')
@click.option('--save-to-disk/--no-save-to-disk', default=True,
              help='Save to filesystem in addition to database')
@click.option('--yes', '-y', is_flag=True, help='Skip confirmation prompt')
def save(project, filename, file_type, description, save_to_disk, yes):
    """Save an artifact from clipboard."""
    try:
        # Auto-detect file type if not specified
        if not file_type:
            ext = Path(filename).suffix.lower()
            if ext in ['.cpp', '.h', '.c', '.hpp', '.py', '.js', '.java']:
                file_type = 'code'
            elif ext in ['.md', '.txt', '.rst']:
                file_type = 'doc'
            elif ext in ['.json', '.yaml', '.yml', '.toml', '.ini']:
                file_type = 'config'
            else:
                file_type = 'other'
        
        # Interactive confirmation (unless --yes flag is used)
        if not yes:
            console.print("\n💾 [bold]Ready to save file from clipboard[/bold]")
            console.print("   [cyan]Project:[/cyan] " + project)
            console.print("   [cyan]Filename:[/cyan] " + filename)
            console.print("   [cyan]Type:[/cyan] " + file_type)
            if description:
                console.print("   [cyan]Description:[/cyan] " + description)
            console.print("\n[yellow]Copy the content to your clipboard, then press Y to save[/yellow]")
            
            response = input("\nSave from clipboard? [Y/n]: ").strip().lower()
            
            if response != "" and response not in ['y', 'yes']:
                console.print("[yellow]✖ Save cancelled[/yellow]")
                return
        
        # Read from clipboard
        console.print("Reading from clipboard...")
        content = pyperclip.paste()
        
        if not content or not content.strip():
            console.print("[red]✗[/red] Clipboard is empty!")
            raise click.Abort()
        
        # Initialize database
        db = KnowledgeDB(DB_PATH)
        
        # Save to database
        artifact_id = db.add_artifact(
            project=project,
            filename=filename,
            content=content,
            file_type=file_type,
            description=description
        )
        
        # Save to filesystem if requested
        if save_to_disk:
            project_dir = AGENT_HOME / "projects" / project / "artifacts"
            project_dir.mkdir(parents=True, exist_ok=True)
            
            file_path = project_dir / filename
            file_path.write_text(content)
        
        # Show success message
        size_kb = len(content.encode()) / 1024
        
        panel_content = f"""[green]✓[/green] Artifact saved successfully

[cyan]Project:[/cyan] {project}
[cyan]File:[/cyan] {filename}
[cyan]Type:[/cyan] {file_type}
[cyan]Size:[/cyan] {size_kb:.1f} KB
[cyan]ID:[/cyan] {artifact_id}"""
        
        if description:
            panel_content += f"\n[cyan]Description:[/cyan] {description}"
        
        if save_to_disk:
            panel_content += f"\n[cyan]Saved to:[/cyan] {project_dir / filename}"
        
        console.print(Panel(panel_content, title="Artifact Saved", border_style="green"))
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()

@cli.command()
def test_template():
    """Test template loader by loading handoff.json."""
    from pathlib import Path
    import json
    from .template_loader import TemplateLoader
    
    try:
        # Initialize loader
        agent_home = Path.home() / "daisy-agent"
        templates_dir = agent_home / "templates"
        loader = TemplateLoader(templates_dir)
        
        console.print("[yellow]Loading template: core/handoff.json[/yellow]")
        
        # Load template
        template = loader.load_template("core/handoff.json")
        
        # Calculate stats
        keys = list(template.keys())
        token_estimate = loader.estimate_tokens(template)
        
        # Display stats
        console.print("\n[green]✅ Template loaded successfully[/green]")
        console.print(f"\n[bold]Stats:[/bold]")
        console.print(f"  • Keys: {len(keys)}")
        console.print(f"  • Token estimate: {token_estimate:,}")
        console.print(f"\n[bold]Top-level keys:[/bold]")
        for key in keys:
            console.print(f"  - {key}")
        
        # Display JSON preview (first 50 lines)
        console.print(f"\n[bold]JSON content (first 50 lines):[/bold]")
        json_str = json.dumps(template, indent=2)
        lines = json_str.split('\n')
        preview_lines = lines[:50]
        
        for line in preview_lines:
            console.print(line)
        
        if len(lines) > 50:
            console.print(f"\n... ({len(lines) - 50} more lines)")
        
    except FileNotFoundError as e:
        console.print(f"[red]❌ Template not found: {e}[/red]")
    except Exception as e:
        console.print(f"[red]❌ Error: {e}[/red]")
        import traceback
        traceback.print_exc()

@cli.command()
@click.option('--project', required=True, help='Project name')
@click.option('--category', 
              type=click.Choice(['lesson', 'pattern', 'gotcha', 'tip', 'decision']),
              help='Note category (auto-detected if not specified)')
@click.option('--scope',
              type=click.Choice(['project', 'global']),
              default='project',
              help='Scope of note: project-specific or globally applicable')
@click.option('--text', help='Note text (or provide as argument)')
@click.argument('note_text', required=False)
@click.option('--yes', '-y', is_flag=True, help='Skip confirmation prompt')
def note(project, category, scope, text, note_text, yes):
    """Capture a session note (lesson, pattern, gotcha, tip, or decision)."""
    try:
        # Get note text from either --text flag or argument
        note_content = text or note_text
        
        if not note_content:
            console.print("[red]✗[/red] Note text is required!")
            console.print("Usage: daisy-agent note --project PROJECT --text 'your note'")
            console.print("   or: daisy-agent note --project PROJECT 'your note'")
            raise click.Abort()
        
        # Auto-detect category if not specified
        if not category:
            note_lower = note_content.lower()
            
            # Simple keyword-based detection
            if any(word in note_lower for word in ['always', 'remember', 'learned', 'discovered']):
                category = 'lesson'
            elif any(word in note_lower for word in ['pattern', 'use', 'implement', 'code']):
                category = 'pattern'
            elif any(word in note_lower for word in ['gotcha', 'careful', 'watch out', 'different', 'hardware']):
                category = 'gotcha'
            elif any(word in note_lower for word in ['optimize', 'faster', 'better', 'improve']):
                category = 'tip'
            elif any(word in note_lower for word in ['decided', 'chose', 'using', 'because']):
                category = 'decision'
            else:
                # Default to lesson
                category = 'lesson'
        
        # Category emojis
        category_emoji = {
            'lesson': '💡',
            'pattern': '🔧',
            'gotcha': '⚠️',
            'tip': '✨',
            'decision': '🎯'
        }
        
        # Initialize database
        db = KnowledgeDB(DB_PATH)
        
        # Save note
        note_id = db.add_note(
            project=project,
            category=category,
            text=note_content,
            scope=scope
        )
        
        # Show success message
        scope_label = "🌍 Global" if scope == "global" else "📦 Project"
        panel_content = f"""[green]✓[/green] Note captured successfully

[cyan]Project:[/cyan] {project}
[cyan]Category:[/cyan] {category_emoji.get(category, '📝')} {category}
[cyan]Scope:[/cyan] {scope_label}
[cyan]ID:[/cyan] {note_id}

[dim]{note_content}[/dim]"""
        
        console.print(Panel(panel_content, title="Note Saved", border_style="green"))
        
        db.close()
        
    except Exception as e:
        console.print(f"[red]✗[/red] Error: {e}")
        raise click.Abort()

@cli.command()
@click.option('--project', required=True, help='Project name')
@click.option('--dry-run', is_flag=True, help='Test without saving to database or file')
def build_handoff(project, dry_run):
    """
    Build handoff document from session-end JSON.
    
    Reads session JSON from clipboard, saves to database,
    generates next handoff, and copies to clipboard.
    """
    from datetime import datetime
    import pyperclip
    import json
    from pathlib import Path
    from .template_loader import TemplateLoader
    from .database import KnowledgeDB
    
    try:
        # 1. Read session-end JSON from clipboard
        console.print("[yellow]📋 Reading session JSON from clipboard...[/yellow]")
        clipboard_content = pyperclip.paste()
        
        if not clipboard_content.strip():
            console.print("[red]❌ Clipboard is empty[/red]")
            return
        
        try:
            session_data = json.loads(clipboard_content)
        except json.JSONDecodeError as e:
            console.print(f"[red]❌ Invalid JSON in clipboard: {e}[/red]")
            console.print("[yellow]Clipboard content preview:[/yellow]")
            console.print(clipboard_content[:200])
            return
        
        # 2. Validate schema
        console.print("[yellow]✓ JSON parsed successfully[/yellow]")
        
        required_keys = ['schema_version', 'project', 'session_summary', 
                        'status', 'mode', 'artifacts', 'lessons', 'next_steps']
        
        missing_keys = [key for key in required_keys if key not in session_data]
        if missing_keys:
            console.print(f"[red]❌ Missing required keys: {missing_keys}[/red]")
            console.print(f"[yellow]Available keys: {list(session_data.keys())}[/yellow]")
            return
        
        # Verify project matches
        if session_data['project'] != project:
            console.print(f"[red]❌ Project mismatch:[/red]")
            console.print(f"   JSON has: '{session_data['project']}'")
            console.print(f"   Expected: '{project}'")
            return
        
        console.print(f"[green]✅ Valid session JSON for project: {project}[/green]")
        
        # 3. Initialize database
        agent_home = Path.home() / "daisy-agent"
        db = KnowledgeDB(agent_home / "knowledge.db")
        
        # 4. Track artifacts
        artifacts_count = len(session_data.get('artifacts', []))
        console.print(f"\n[yellow]📄 Tracking {artifacts_count} artifacts:[/yellow]")
        for artifact in session_data.get('artifacts', []):
            console.print(f"   • {artifact['file']} ({artifact['type']})")
        
        # 5. Save lessons to database
        console.print(f"\n[yellow]💡 Saving lessons to knowledge base:[/yellow]")
        lessons_saved = 0
        
        if not dry_run:
            for lesson in session_data.get('lessons', []):
                scope = lesson.get('scope', 'project')
                category = lesson.get('category', 'lesson')
                text = lesson['text']
                
                db.add_note(
                    project=project,
                    category=category,
                    text=text,
                    scope=scope
                )
                
                scope_emoji = "🌍" if scope == "global" else "📦"
                console.print(f"   {scope_emoji} [{category}] {text[:60]}...")
                lessons_saved += 1
        else:
            console.print("[yellow]   (dry-run: skipping database save)[/yellow]")
            lessons_saved = len(session_data.get('lessons', []))
        
        console.print(f"[green]✅ Saved {lessons_saved} lessons[/green]")
        
        if dry_run:
            console.print("\n[yellow]⚠️  Dry run mode - stopping before handoff generation[/yellow]")
            db.close()
            return
        
        # 6. Query database for context
        console.print("\n[yellow]🔍 Gathering context from knowledge base...[/yellow]")
        
        # Get project and global lessons (limit for token budget)
        project_lessons_raw = db.get_notes(project=project, scope='project')
        global_lessons_raw = db.get_notes(project=project, scope='global')
        
        # Format for template
        project_lessons = [
            {"category": note['category'], "text": note['text']}
            for note in project_lessons_raw[:20]  # Limit to prevent token overflow
        ]
        
        global_lessons = [
            {"category": note['category'], "text": note['text']}
            for note in global_lessons_raw[:10]
        ]
        
        console.print(f"   📦 {len(project_lessons)} project lessons (max 20)")
        console.print(f"   🌍 {len(global_lessons)} global lessons (max 10)")
        
        db.close()
        
        # 7. Load and process handoff template
        console.print("\n[yellow]📝 Generating handoff document...[/yellow]")
        
        templates_dir = agent_home / "templates"
        loader = TemplateLoader(templates_dir)
        
        # Prepare variables
        timestamp = datetime.now().isoformat()
        
        variables = {
            "timestamp": timestamp,
            "project": project,
            "platform": session_data.get('platform', 'unknown'),  # ✅ This stays - used by template
            "session_number": session_data.get('session_number', 1),
            "mode": session_data['mode'],
            "last_working_code": session_data.get('last_working_code', ''),
            "current_work": session_data.get('current_work', ''),
            "status": session_data['status'],
            "session_summary": session_data['session_summary'],
            "key_artifacts": session_data['artifacts'][:5],              # ✅ FIXED - removed json.dumps()
            "key_lessons": session_data['lessons'][:5],                  # ✅ FIXED - removed json.dumps()
            "recent_artifacts": session_data['artifacts'],               # ✅ FIXED - removed json.dumps()
            "project_lessons": project_lessons,                          # ✅ FIXED - removed json.dumps()
            "global_lessons": global_lessons,                            # ✅ FIXED - removed json.dumps()
            "project_patterns": [],                                      # ✅ FIXED - actual empty array not string
            "next_focus": session_data['next_steps'],
            "next_mode": "GUIDED",
            "suggested_steps": session_data.get('suggested_steps', session_data['next_steps']),
            "blockers": session_data.get('blockers', [])                 # ✅ FIXED - removed json.dumps()
        }
        
        # Process template with token budget
        result = loader.load_and_process(
            template_path="core/handoff.json",
            variables=variables,
            token_budget=8000,
            truncate_keys=["recent_artifacts", "project_lessons", "global_lessons"]
        )
        
        console.print(f"[green]✅ Generated handoff: {result.tokens_used:,} tokens[/green]")
        
        if result.truncated_sections:
            console.print("[yellow]⚠️  Truncated sections to meet token budget:[/yellow]")
            for section in result.truncated_sections:
                console.print(f"   • {section}")
        
        # 8. Save handoff to file
        handoffs_dir = agent_home / "handoffs"
        handoffs_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate filename
        timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
        handoff_filename = f"{project}_{timestamp_str}.json"
        handoff_path = handoffs_dir / handoff_filename
        
        with open(handoff_path, 'w', encoding='utf-8') as f:
            json.dump(result.content, f, indent=2)
        
        console.print(f"\n[green]💾 Saved: {handoff_path}[/green]")
        
        # 9. Copy to clipboard
        handoff_json = json.dumps(result.content, indent=2)
        pyperclip.copy(handoff_json)
        
        console.print("[green]📋 Copied handoff JSON to clipboard[/green]")
        
        # 10. Display summary
        from rich.panel import Panel
        
        summary = f"""[bold green]Handoff Generated Successfully[/bold green]

📊 Statistics:
   • Tokens: {result.tokens_used:,} / 8,000
   • Variables: {result.variables_substituted}
   • Artifacts: {artifacts_count}
   • Lessons: {lessons_saved} saved ({len(project_lessons)} project + {len(global_lessons)} global)

📁 Output:
   • File: {handoff_path.name}
   • Location: {handoffs_dir}

📋 Next Steps:
   1. Start new Claude session
   2. Paste JSON from clipboard
   3. Continue work in {session_data['mode']} mode
"""
        
        console.print(Panel(summary, border_style="green", title="Build Complete"))
        
    except Exception as e:
        console.print(f"\n[red]❌ Error: {e}[/red]")
        import traceback
        traceback.print_exc()

if __name__ == '__main__':
    cli()
