"""
Handoff document generator for daisy-agent.
Creates session handoff documents with compressed context, artifacts, and notes.
"""

import pyperclip
from pathlib import Path
from datetime import datetime
from .database import KnowledgeDB
from .compressor import ContextCompressor


class HandoffGenerator:
    """Generates handoff documents for session transitions."""
    
    def __init__(self, db: KnowledgeDB):
        """Initialize with database connection."""
        self.db = db
        self.compressor = ContextCompressor(db)
        
    def generate_handoff(self, project: str, current_work: str, 
                        working_file: str = None) -> Path:
        """
        Generate a handoff document for session transition.
        
        Args:
            project: Project name
            current_work: Description of current work
            working_file: Optional working file name
            
        Returns:
            Path to generated handoff file
        """
        # Generate timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M")
        
        # Get artifacts and notes
        artifacts = self.db.get_artifacts(project)
        notes = self.db.get_notes(project)
        notes_by_cat = self.db.get_notes_by_category(project)
        
        # Compress context for this project
        query = f"{project} {current_work}"
        compressed_context = self.compressor.compress_context(query, max_tokens=2000)
        stats = self.compressor.get_compression_stats(query, max_tokens=2000)
        
        # Build handoff document
        doc_parts = []
        
        # Header
        doc_parts.append(f"# Session Handoff: {project} - {datetime.now().strftime('%Y-%m-%d %H:%M')}\n\n")
        
        # Session info
        doc_parts.append("## SESSION INFO\n\n")
        doc_parts.append(f"**Mode:** HANDOFF\n")
        doc_parts.append(f"**Project:** {project}\n")
        doc_parts.append(f"**Current Work:** {current_work}\n")
        if working_file:
            doc_parts.append(f"**Working Code:** {working_file}\n")
        doc_parts.append("\n")
        
        # Artifacts section
        if artifacts:
            doc_parts.append(f"## ARTIFACTS TO SAVE ({len(artifacts)} file{'s' if len(artifacts) != 1 else ''})\n\n")
            
            for art in artifacts:
                size_kb = art['size_bytes'] / 1024
                file_type = art['file_type'] or 'unknown'
                
                doc_parts.append(f"### {art['filename']} ({size_kb:.1f} KB, {file_type})\n")
                if art['description']:
                    doc_parts.append(f"{art['description']}\n")
                doc_parts.append("\n")
            
            # Save commands section
            doc_parts.append("**Quick Save Commands:**\n\n")
            doc_parts.append("```bash\n")
            for art in artifacts:
                doc_parts.append(f"# Copy {art['filename']} from artifact above, then:\n")
                doc_parts.append(f"daisy-agent save --project {project} --file {art['filename']}")
                if art['description']:
                    # Escape single quotes in description
                    desc = art['description'].replace("'", "'\\''")
                    doc_parts.append(f" \\\n  --description '{desc}'")
                doc_parts.append("\n\n")
            doc_parts.append("```\n\n")
        
        # Notes section
        if notes:
            doc_parts.append(f"## LESSONS LEARNED ({len(notes)} note{'s' if len(notes) != 1 else ''})\n\n")
            
            category_emoji = {
                'lesson': '💡',
                'pattern': '🔧',
                'gotcha': '⚠️',
                'tip': '✨',
                'decision': '🎯'
            }
            
            category_names = {
                'lesson': 'Lessons',
                'pattern': 'Patterns',
                'gotcha': 'Gotchas',
                'tip': 'Tips',
                'decision': 'Decisions'
            }
            
            for category, cat_notes in notes_by_cat.items():
                if cat_notes:
                    emoji = category_emoji.get(category, '📝')
                    name = category_names.get(category, category.capitalize())
                    doc_parts.append(f"### {emoji} {name}\n")
                    for note in cat_notes:
                        doc_parts.append(f"- {note['text']}\n")
                    doc_parts.append("\n")
            
            # Capture commands section
            doc_parts.append("**Capture These Notes:**\n\n")
            doc_parts.append("```bash\n")
            for category, cat_notes in notes_by_cat.items():
                if cat_notes:
                    for note in cat_notes:
                        # Escape single quotes
                        text = note['text'].replace("'", "'\\''")
                        doc_parts.append(f"daisy-agent note --project {project} \\\n")
                        doc_parts.append(f"  --category {category} \\\n")
                        doc_parts.append(f"  --text '{text}'\n\n")
            doc_parts.append("```\n\n")
        
        # Compressed context
        doc_parts.append(f"## COMPRESSED CONTEXT ({stats['compressed_tokens']:,} tokens)\n\n")
        doc_parts.append(compressed_context)
        doc_parts.append("\n")
        
        # Next session
        doc_parts.append("## NEXT SESSION\n\n")
        doc_parts.append("When starting the next session, use GUIDED mode for continued development.\n\n")
        
        # Summary of what's been captured
        if artifacts or notes:
            doc_parts.append("### Session Captured\n")
            if artifacts:
                doc_parts.append(f"- ✅ {len(artifacts)} artifact{'s' if len(artifacts) != 1 else ''}\n")
            if notes:
                doc_parts.append(f"- ✅ {len(notes)} note{'s' if len(notes) != 1 else ''}\n")
            doc_parts.append("\n")
        
        # Footer
        doc_parts.append("---\n")
        doc_parts.append("*Generated by daisy-agent v0.2.0*\n")
        
        handoff_content = ''.join(doc_parts)
        
        # Save to file
        handoff_dir = Path.home() / "daisy-agent" / "handoffs"
        handoff_dir.mkdir(parents=True, exist_ok=True)
        
        handoff_filename = f"{project}_{timestamp}.md"
        handoff_path = handoff_dir / handoff_filename
        
        handoff_path.write_text(handoff_content)
        
        # Copy to clipboard
        pyperclip.copy(handoff_content)
        
        return handoff_path
