"""
Context compression engine for daisy-agent.
Handles knowledge indexing and compression for Claude sessions.
"""

import json
import hashlib
from pathlib import Path
from typing import List, Dict, Tuple
from .database import KnowledgeDB


class ContextCompressor:
    """Compresses project knowledge for efficient Claude sessions."""
    
    def __init__(self, db: KnowledgeDB):
        """Initialize with database connection."""
        self.db = db
        
    def estimate_tokens(self, text: str) -> int:
        """Conservative token estimation (4 chars per token)."""
        return len(text) // 4
        
    def create_embedding_hash(self, text: str) -> str:
        """Create hash-based embedding for semantic similarity."""
        # Normalize text
        normalized = text.lower().strip()
        
        # Extract keywords (words > 3 chars)
        words = [w for w in normalized.split() if len(w) > 3]
        
        # Create hash from sorted unique keywords
        keyword_set = sorted(set(words))
        return hashlib.md5(''.join(keyword_set).encode()).hexdigest()

    def calculate_jaccard_similarity(self, set1: set, set2: set) -> float:
        """Calculate Jaccard similarity between two sets."""
        if not set1 or not set2:
            return 0.0
        intersection = set1 & set2
        union = set1 | set2
        return len(intersection) / len(union)

    def extract_keywords(self, text: str) -> set:
        """Extract keywords from text (words > 3 chars)."""
        normalized = text.lower().strip()
        words = [w for w in normalized.split() if len(w) > 3]
        return set(words)
        
    def chunk_document(self, source: str, content: Dict) -> List[Dict]:
        """Split document into semantic chunks."""
        chunks = []
        
        if isinstance(content, dict):
            # JSON documents - chunk by top-level keys
            for key, value in content.items():
                if isinstance(value, (dict, list)):
                    value_str = json.dumps(value, indent=2)
                else:
                    value_str = str(value)
                
                chunk = {
                    'source': source,
                    'section': key,
                    'content': value_str,
                    'tokens': self.estimate_tokens(value_str),
                    'embedding_hash': self.create_embedding_hash(value_str)
                }
                chunks.append(chunk)
        else:
            # Plain text - single chunk
            content_str = str(content)
            chunk = {
                'source': source,
                'section': 'content',
                'content': content_str,
                'tokens': self.estimate_tokens(content_str),
                'embedding_hash': self.create_embedding_hash(content_str)
            }
            chunks.append(chunk)
            
        return chunks
        
    def index_file(self, filepath: Path) -> int:
        """
        Index a file into the knowledge base.
        
        Args:
            filepath: Path to file to index
            
        Returns:
            Number of chunks indexed
        """
        if not filepath.exists():
            raise FileNotFoundError(f"File not found: {filepath}")
            
        # Clear existing chunks from this source
        self.db.clear_source(filepath.name)
            
        # Read file based on extension
        if filepath.suffix == '.json':
            with open(filepath, 'r') as f:
                content = json.load(f)
        else:
            with open(filepath, 'r') as f:
                content = f.read()
                
        # Chunk the document
        chunks = self.chunk_document(filepath.name, content)
        
        # Insert chunks into database
        for chunk in chunks:
            self.db.add_chunk(
                source=chunk['source'],
                section=chunk['section'],
                content=chunk['content'],
                tokens=chunk['tokens'],
                embedding_hash=chunk['embedding_hash']
            )
            
        return len(chunks)
        
    def compress_context(self, query: str, max_tokens: int = 2000) -> str:
        """
        Generate compressed context for a query with relevance ranking.
        
        Args:
            query: The query to find relevant context for
            max_tokens: Maximum tokens in compressed output
            
        Returns:
            Compressed context string
        """
        # Extract query keywords
        query_keywords = self.extract_keywords(query)
    
        # Get all chunks
        all_chunks = self.db.get_all_chunks()
    
        # Calculate relevance score for each chunk
        scored_chunks = []
        for chunk in all_chunks:
            chunk_keywords = self.extract_keywords(chunk['content'])
            similarity = self.calculate_jaccard_similarity(query_keywords, chunk_keywords)
            scored_chunks.append((similarity, chunk))
    
        # Sort by relevance (highest first)
        scored_chunks.sort(key=lambda x: x[0], reverse=True)
    
        # Build compressed context with most relevant chunks
        context_parts = [f"# Task: {query}\n\n"]
        context_parts.append("# Relevant Context\n\n")
    
        token_count = self.estimate_tokens(''.join(context_parts))
    
        for similarity, chunk in scored_chunks:
            # Skip chunks with zero relevance
            if similarity == 0:
                continue
            
            chunk_tokens = chunk['tokens']
            if token_count + chunk_tokens > max_tokens:
                break
            
            context_parts.append(f"## {chunk['source']} - {chunk['section']}\n\n")
            context_parts.append(chunk['content'])
            context_parts.append("\n\n")
        
            token_count += chunk_tokens
        
        compressed_context = ''.join(context_parts)
        return compressed_context
                
    def get_compression_stats(self, query: str, max_tokens: int = 2000) -> Dict:
        """
        Get statistics about compression for a query.
        
        Args:
            query: The query to analyze
            max_tokens: Maximum tokens for compression
            
        Returns:
            Dict with compression statistics
        """
        compressed = self.compress_context(query, max_tokens)
        compressed_tokens = self.estimate_tokens(compressed)
        
        stats = self.db.get_stats()
        full_tokens = stats['total_tokens']
        
        compression_ratio = 1.0 - (compressed_tokens / full_tokens) if full_tokens > 0 else 0.0
        
        return {
            'compressed_tokens': compressed_tokens,
            'full_tokens': full_tokens,
            'compression_ratio': compression_ratio,
            'compression_pct': compression_ratio * 100
        }
