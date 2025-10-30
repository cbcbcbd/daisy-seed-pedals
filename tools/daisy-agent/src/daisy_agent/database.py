"""
Database module for daisy-agent knowledge base.
Handles SQLite operations for chunks, artifacts, notes, and projects.
"""

import sqlite3
import hashlib
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Optional, Tuple


class KnowledgeDB:
    """SQLite database for project knowledge, artifacts, and session notes."""
    
    def __init__(self, db_path: Path):
        """Initialize database connection."""
        self.db_path = db_path
        self.conn = None
        self._connect()
        self._init_schema()
    
    def _connect(self):
        """Create database connection."""
        self.conn = sqlite3.connect(self.db_path)
        self.conn.row_factory = sqlite3.Row  # Access columns by name
    
    def _init_schema(self):
        """Initialize or update database schema."""
        cursor = self.conn.cursor()
        
        # Original chunks table (Phase 0)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS chunks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                source TEXT NOT NULL,
                section TEXT NOT NULL,
                content TEXT NOT NULL,
                tokens INTEGER NOT NULL,
                embedding_hash TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_source ON chunks(source)")
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_embedding ON chunks(embedding_hash)")
        
        # New artifacts table (Phase 1)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS artifacts (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                project TEXT NOT NULL,
                filename TEXT NOT NULL,
                content TEXT NOT NULL,
                content_hash TEXT NOT NULL,
                file_type TEXT,
                description TEXT,
                size_bytes INTEGER,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_artifact_project ON artifacts(project)")
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_artifact_hash ON artifacts(content_hash)")
        
        # New notes table (Phase 1)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS notes (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                project TEXT NOT NULL,
                category TEXT NOT NULL,
                text TEXT NOT NULL,
                scope TEXT DEFAULT 'project',
                status TEXT DEFAULT 'active',
                confidence TEXT DEFAULT 'low',
                sessions_confirmed INTEGER DEFAULT 1,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_note_project ON notes(project)")
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_note_category ON notes(category)")
        cursor.execute("CREATE INDEX IF NOT EXISTS idx_note_scope ON notes(scope)")
        
        # New projects table (Phase 1)
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS projects (
                name TEXT PRIMARY KEY,
                platform TEXT,
                last_session TIMESTAMP,
                total_sessions INTEGER DEFAULT 0,
                last_working_file TEXT,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        self.conn.commit()
    
    # ==================== CHUNK METHODS (Phase 0) ====================
    
    def add_chunk(self, source: str, section: str, content: str, tokens: int, embedding_hash: str):
        """Add a knowledge chunk to the database."""
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO chunks (source, section, content, tokens, embedding_hash)
            VALUES (?, ?, ?, ?, ?)
        """, (source, section, content, tokens, embedding_hash))
        self.conn.commit()
    
    def get_chunks_by_embedding(self, embedding_hash: str, limit: int = 10) -> List[Dict]:
        """Retrieve chunks matching an embedding hash."""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT source, section, content, tokens
            FROM chunks
            WHERE embedding_hash = ?
            LIMIT ?
        """, (embedding_hash, limit))
        
        return [dict(row) for row in cursor.fetchall()]
    
    def get_all_chunks(self) -> List[Dict]:
        """Retrieve all chunks for relevance scoring."""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT id, source, section, content, tokens, embedding_hash
            FROM chunks
        """)
        
        return [dict(row) for row in cursor.fetchall()]
    
    def get_stats(self) -> Dict:
        """Get database statistics."""
        cursor = self.conn.cursor()
        
        # Chunk stats
        cursor.execute("SELECT COUNT(*) as count FROM chunks")
        chunk_count = cursor.fetchone()['count']
        
        cursor.execute("SELECT SUM(tokens) as total FROM chunks")
        total_tokens = cursor.fetchone()['total'] or 0
        
        cursor.execute("SELECT COUNT(DISTINCT source) as count FROM chunks")
        source_count = cursor.fetchone()['count']
        
        # Artifact stats
        cursor.execute("SELECT COUNT(*) as count FROM artifacts")
        artifact_count = cursor.fetchone()['count']
        
        cursor.execute("SELECT COUNT(DISTINCT project) as count FROM artifacts")
        artifact_projects = cursor.fetchone()['count']
        
        # Note stats
        cursor.execute("SELECT COUNT(*) as count FROM notes")
        note_count = cursor.fetchone()['count']
        
        cursor.execute("SELECT COUNT(DISTINCT project) as count FROM notes")
        note_projects = cursor.fetchone()['count']
        
        # Project stats
        cursor.execute("SELECT COUNT(*) as count FROM projects")
        project_count = cursor.fetchone()['count']
        
        return {
            'chunks': chunk_count,
            'total_tokens': total_tokens,
            'sources': source_count,
            'artifacts': artifact_count,
            'artifact_projects': artifact_projects,
            'notes': note_count,
            'note_projects': note_projects,
            'projects': project_count
        }
    
    def clear_source(self, source: str):
        """Remove all chunks from a specific source (for re-indexing)."""
        cursor = self.conn.cursor()
        cursor.execute("DELETE FROM chunks WHERE source = ?", (source,))
        self.conn.commit()
    
    # ==================== ARTIFACT METHODS (Phase 1) ====================
    
    def add_artifact(self, project: str, filename: str, content: str, 
                    file_type: Optional[str] = None,
                    description: Optional[str] = None) -> int:
        """
        Add an artifact to the database.
        
        Args:
            project: Project name
            filename: Name of the file
            content: File content
            file_type: Type of file (code, doc, config, etc.)
            description: Optional description
            
        Returns:
            ID of the inserted artifact
        """
        content_hash = hashlib.md5(content.encode()).hexdigest()
        size_bytes = len(content.encode())
        
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO artifacts (project, filename, content, content_hash, 
                                 file_type, description, size_bytes)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (project, filename, content, content_hash, file_type, description, size_bytes))
        
        self.conn.commit()
        
        # Update project metadata
        self._update_project_session(project, filename)
        
        return cursor.lastrowid
    
    def get_artifacts(self, project: str, 
                     since: Optional[datetime] = None) -> List[Dict]:
        """
        Retrieve artifacts for a project.
        
        Args:
            project: Project name
            since: Optional timestamp to get artifacts since
            
        Returns:
            List of artifact dictionaries
        """
        cursor = self.conn.cursor()
        
        if since:
            cursor.execute("""
                SELECT id, filename, content, file_type, description, 
                       size_bytes, created_at
                FROM artifacts
                WHERE project = ? AND created_at >= ?
                ORDER BY created_at DESC
            """, (project, since))
        else:
            cursor.execute("""
                SELECT id, filename, content, file_type, description, 
                       size_bytes, created_at
                FROM artifacts
                WHERE project = ?
                ORDER BY created_at DESC
            """, (project,))
        
        return [dict(row) for row in cursor.fetchall()]
    
    def get_artifact_by_id(self, artifact_id: int) -> Optional[Dict]:
        """Retrieve a specific artifact by ID."""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT id, project, filename, content, file_type, description,
                   size_bytes, created_at
            FROM artifacts
            WHERE id = ?
        """, (artifact_id,))
        
        row = cursor.fetchone()
        return dict(row) if row else None
    
    # ==================== NOTE METHODS (Phase 1) ====================
    
    def add_note(self, project: str, category: str, text: str,
                confidence: str = 'low', scope: str = 'project') -> int:
        """
        Add a session note.
        
        Args:
            project: Project name
            category: Note category (lesson, pattern, gotcha, tip, decision)
            text: Note text
            confidence: Confidence level (low, medium, high)
            scope: Scope of note (project, global)
            
        Returns:
            ID of the inserted note
        """
        valid_categories = ['lesson', 'pattern', 'gotcha', 'tip', 'decision']
        if category not in valid_categories:
            raise ValueError(f"Category must be one of: {', '.join(valid_categories)}")
        
        valid_confidence = ['low', 'medium', 'high']
        if confidence not in valid_confidence:
            raise ValueError(f"Confidence must be one of: {', '.join(valid_confidence)}")
        
        valid_scope = ['project', 'global']
        if scope not in valid_scope:
            raise ValueError(f"Scope must be one of: {', '.join(valid_scope)}")
        
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO notes (project, category, text, confidence, scope)
            VALUES (?, ?, ?, ?, ?)
        """, (project, category, text, confidence, scope))
        
        self.conn.commit()
        
        # Update project metadata
        self._update_project_session(project)
        
        return cursor.lastrowid
    
    def get_notes(self, project: str, 
                 category: Optional[str] = None,
                 scope: Optional[str] = None,
                 since: Optional[datetime] = None) -> List[Dict]:
        """
        Retrieve notes for a project.
        
        Args:
            project: Project name
            category: Optional filter by category
            scope: Optional filter by scope (project, global)
            since: Optional timestamp to get notes since
            
        Returns:
            List of note dictionaries
        """
        cursor = self.conn.cursor()
        
        query = """
            SELECT id, category, text, confidence, scope, status, 
                   sessions_confirmed, created_at
            FROM notes
            WHERE project = ?
        """
        params = [project]
        
        if category:
            query += " AND category = ?"
            params.append(category)
        
        if scope:
            query += " AND scope = ?"
            params.append(scope)
        
        if since:
            query += " AND created_at >= ?"
            params.append(since)
        
        query += " ORDER BY created_at DESC"
        
        cursor.execute(query, params)
        
        return [dict(row) for row in cursor.fetchall()]
    
    def get_notes_by_category(self, project: str) -> Dict[str, List[Dict]]:
        """Get notes organized by category."""
        notes = self.get_notes(project)
        
        by_category = {
            'lesson': [],
            'pattern': [],
            'gotcha': [],
            'tip': [],
            'decision': []
        }
        
        for note in notes:
            by_category[note['category']].append(note)
        
        return by_category
    
    # ==================== PROJECT METHODS (Phase 1) ====================
    
    def get_project_info(self, project: str) -> Optional[Dict]:
        """Get project metadata."""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT name, platform, last_session, total_sessions, 
                   last_working_file, created_at
            FROM projects
            WHERE name = ?
        """, (project,))
        
        row = cursor.fetchone()
        return dict(row) if row else None
    
    def create_project(self, name: str, platform: Optional[str] = None) -> str:
        """
        Create a new project entry.
        
        Args:
            name: Project name
            platform: Optional platform (hothouse, funbox, terrarium, etc.)
            
        Returns:
            Project name
        """
        cursor = self.conn.cursor()
        cursor.execute("""
            INSERT INTO projects (name, platform, total_sessions)
            VALUES (?, ?, 0)
            ON CONFLICT(name) DO NOTHING
        """, (name, platform))
        
        self.conn.commit()
        return name
    
    def _update_project_session(self, project: str, 
                               working_file: Optional[str] = None):
        """Update project metadata during session activity."""
        cursor = self.conn.cursor()
        
        # Create project if it doesn't exist
        cursor.execute("""
            INSERT INTO projects (name, last_session, total_sessions)
            VALUES (?, CURRENT_TIMESTAMP, 1)
            ON CONFLICT(name) DO UPDATE SET
                last_session = CURRENT_TIMESTAMP
        """, (project,))
        
        # Update working file if provided
        if working_file:
            cursor.execute("""
                UPDATE projects
                SET last_working_file = ?
                WHERE name = ?
            """, (working_file, project))
        
        self.conn.commit()
    
    def list_projects(self) -> List[Dict]:
        """List all projects with metadata."""
        cursor = self.conn.cursor()
        cursor.execute("""
            SELECT name, platform, last_session, total_sessions, last_working_file
            FROM projects
            ORDER BY last_session DESC
        """)
        
        return [dict(row) for row in cursor.fetchall()]
    
    # ==================== CLEANUP ====================
    
    def close(self):
        """Close database connection."""
        if self.conn:
            self.conn.close()
