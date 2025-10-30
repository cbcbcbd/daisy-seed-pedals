"""
Template Loader Module for daisy-agent

Handles loading JSON templates, variable substitution, and token budget constraints.
"""

import json
import re
from pathlib import Path
from typing import Dict, Any, List, Optional
from dataclasses import dataclass


@dataclass
class TemplateLoadResult:
    """Result of loading and processing a template."""
    content: Dict[str, Any]
    tokens_used: int
    variables_substituted: int
    truncated_sections: List[str]


class TemplateLoader:
    """Loads and processes JSON templates with variable substitution."""
    
    def __init__(self, templates_dir: Path):
        """
        Initialize template loader.
        
        Args:
            templates_dir: Path to templates directory (e.g., ~/daisy-agent/templates)
        """
        self.templates_dir = Path(templates_dir)
        self.templates_dir.mkdir(parents=True, exist_ok=True)
        
    def load_template(self, template_path: str) -> Dict[str, Any]:
        """
        Load a JSON template file.
        
        Args:
            template_path: Relative path from templates_dir (e.g., "core/handoff.json")
            
        Returns:
            Parsed JSON template as dictionary
            
        Raises:
            FileNotFoundError: If template doesn't exist
            json.JSONDecodeError: If template is invalid JSON
        """
        full_path = self.templates_dir / template_path
        
        if not full_path.exists():
            raise FileNotFoundError(f"Template not found: {full_path}")
        
        with open(full_path, 'r', encoding='utf-8') as f:
            return json.load(f)
    
    def substitute_variables(
        self,
        template: Dict[str, Any],
        variables: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Recursively substitute {variable} placeholders in template.
        
        Args:
            template: Template dictionary with {var} placeholders
            variables: Dictionary of variable names to values
            
        Returns:
            New dictionary with variables substituted
        """
        # Track which variables were used
        substituted = set()
        
        def substitute_recursive(obj):
            """Recursively process object and substitute variables."""
            if isinstance(obj, str):
                # String: look for {variable} pattern
                pattern = r'\{([a-zA-Z_][a-zA-Z0-9_]*)\}'
                matches = list(re.finditer(pattern, obj))
                
                if not matches:
                    return obj
                
                # If entire string is a single {var}, replace with actual value
                if len(matches) == 1 and matches[0].group(0) == obj:
                    var_name = matches[0].group(1)
                    if var_name in variables:
                        substituted.add(var_name)
                        return variables[var_name]  # Return actual value (list, dict, etc.)
                    return obj
                
                # Multiple variables or mixed with text: string substitution
                result = obj
                for match in reversed(matches):  # Reverse to maintain positions
                    var_name = match.group(1)
                    if var_name in variables:
                        substituted.add(var_name)
                        value = variables[var_name]
                        # Convert to string
                        if isinstance(value, (dict, list)):
                            replacement = json.dumps(value)
                        else:
                            replacement = str(value)
                        result = result[:match.start()] + replacement + result[match.end():]
                
                return result
                
            elif isinstance(obj, dict):
                # Dictionary: recursively process all values
                return {key: substitute_recursive(value) for key, value in obj.items()}
                
            elif isinstance(obj, list):
                # List: recursively process all items
                return [substitute_recursive(item) for item in obj]
                
            else:
                # Other types: return as-is
                return obj
        
        # Process template
        result = substitute_recursive(template)
        
        # Add metadata
        if isinstance(result, dict):
            result['_meta'] = {
                'variables_substituted': len(substituted),
                'substituted_vars': list(substituted)
            }
        
        return result
    
    def estimate_tokens(self, data: Any) -> int:
        """
        Estimate token count for data.
        
        Uses conservative estimate: 1 token ≈ 4 characters
        
        Args:
            data: Any JSON-serializable data
            
        Returns:
            Estimated token count
        """
        if isinstance(data, str):
            # String: count characters
            return len(data) // 4 + 1
        elif isinstance(data, (int, float, bool)):
            # Numbers/booleans: minimal tokens
            return 1
        elif isinstance(data, list):
            # List: sum of items plus structure tokens
            return sum(self.estimate_tokens(item) for item in data) + len(data)
        elif isinstance(data, dict):
            # Dict: sum of keys and values plus structure tokens
            total = 0
            for key, value in data.items():
                if key.startswith('_'):  # Skip metadata keys
                    continue
                total += len(key) // 4 + 1  # Key tokens
                total += self.estimate_tokens(value)  # Value tokens
            return total + len(data)  # Add structure overhead
        else:
            # Default: convert to string and estimate
            return len(str(data)) // 4 + 1
    
    def apply_token_budget(
        self,
        data: Dict[str, Any],
        budget: int,
        truncate_keys: Optional[List[str]] = None
    ) -> tuple[Dict[str, Any], List[str]]:
        """
        Apply token budget constraints by truncating specified sections.
        
        Args:
            data: Dictionary to apply budget to
            budget: Maximum tokens allowed
            truncate_keys: Keys that can be truncated (e.g., ["recent_artifacts", "lessons"])
            
        Returns:
            Tuple of (truncated_data, list_of_truncated_sections)
        """
        if truncate_keys is None:
            truncate_keys = []
        
        # Make a copy to avoid modifying original
        result = json.loads(json.dumps(data))
        truncated_sections = []
        
        # Calculate current token usage
        current_tokens = self.estimate_tokens(result)
        
        if current_tokens <= budget:
            return result, truncated_sections
        
        # Need to truncate - process each truncatable key
        for key in truncate_keys:
            if current_tokens <= budget:
                break
                
            if key not in result:
                continue
            
            original_value = result[key]
            
            # Handle lists by removing items
            if isinstance(original_value, list) and original_value:
                original_length = len(original_value)
                
                # Binary search for right number of items
                while len(result[key]) > 0 and current_tokens > budget:
                    # Remove last item
                    result[key].pop()
                    current_tokens = self.estimate_tokens(result)
                
                if len(result[key]) < original_length:
                    truncated_sections.append(f"{key}: {original_length} → {len(result[key])} items")
            
            # Handle strings by truncating
            elif isinstance(original_value, str) and original_value:
                original_length = len(original_value)
                
                # Binary search for right length
                while len(result[key]) > 100 and current_tokens > budget:
                    # Truncate by 20%
                    new_length = int(len(result[key]) * 0.8)
                    result[key] = result[key][:new_length] + "..."
                    current_tokens = self.estimate_tokens(result)
                
                if len(result[key]) < original_length:
                    truncated_sections.append(f"{key}: {original_length} → {len(result[key])} chars")
        
        return result, truncated_sections
    
    def load_and_process(
        self,
        template_path: str,
        variables: Dict[str, Any],
        token_budget: Optional[int] = None,
        truncate_keys: Optional[List[str]] = None
    ) -> TemplateLoadResult:
        """
        Load template, substitute variables, and apply token budget in one call.
        
        Args:
            template_path: Relative path to template (e.g., "core/handoff.json")
            variables: Dictionary of variables to substitute
            token_budget: Optional maximum tokens (None = no limit)
            truncate_keys: Keys that can be truncated to meet budget
            
        Returns:
            TemplateLoadResult with processed content and metadata
        """
        # Load template
        template = self.load_template(template_path)
        
        # Substitute variables
        processed = self.substitute_variables(template, variables)
        
        # Apply token budget if specified
        truncated_sections = []
        if token_budget is not None:
            processed, truncated_sections = self.apply_token_budget(
                processed,
                token_budget,
                truncate_keys
            )
        
        # Calculate final token count
        tokens_used = self.estimate_tokens(processed)
        
        # Get substitution count from metadata
        variables_substituted = processed.get('_meta', {}).get('variables_substituted', 0)
        
        # Remove metadata from final output
        if '_meta' in processed:
            del processed['_meta']
        
        return TemplateLoadResult(
            content=processed,
            tokens_used=tokens_used,
            variables_substituted=variables_substituted,
            truncated_sections=truncated_sections
        )
    
    def merge_templates(
        self,
        base_template_path: str,
        overlay_template_paths: List[str],
        variables: Dict[str, Any]
    ) -> Dict[str, Any]:
        """
        Load and merge multiple templates together.
        
        Base template is loaded first, then overlays are applied in order.
        Later overlays override earlier values for duplicate keys.
        
        Args:
            base_template_path: Path to base template
            overlay_template_paths: List of overlay template paths
            variables: Variables to substitute in all templates
            
        Returns:
            Merged and processed template dictionary
        """
        # Load base template
        result = self.load_template(base_template_path)
        
        # Apply overlays
        for overlay_path in overlay_template_paths:
            overlay = self.load_template(overlay_path)
            result = self._deep_merge(result, overlay)
        
        # Substitute variables in merged result
        result = self.substitute_variables(result, variables)
        
        return result
    
    def _deep_merge(self, base: Dict, overlay: Dict) -> Dict:
        """
        Deep merge two dictionaries.
        
        Args:
            base: Base dictionary
            overlay: Dictionary to merge into base
            
        Returns:
            New dictionary with overlay merged into base
        """
        result = base.copy()
        
        for key, value in overlay.items():
            if key in result and isinstance(result[key], dict) and isinstance(value, dict):
                # Recursively merge nested dicts
                result[key] = self._deep_merge(result[key], value)
            else:
                # Override with overlay value
                result[key] = value
        
        return result
    
    def list_templates(self, subdirectory: str = "") -> List[str]:
        """
        List all available templates in a subdirectory.
        
        Args:
            subdirectory: Subdirectory to search (e.g., "core", "platforms")
            
        Returns:
            List of template paths relative to templates_dir
        """
        search_dir = self.templates_dir / subdirectory if subdirectory else self.templates_dir
        
        if not search_dir.exists():
            return []
        
        templates = []
        for template_file in search_dir.rglob("*.json"):
            # Get path relative to templates_dir
            rel_path = template_file.relative_to(self.templates_dir)
            templates.append(str(rel_path))
        
        return sorted(templates)


# Convenience functions for common operations

def load_handoff_template(
    templates_dir: Path,
    project: str,
    artifacts: List[Dict],
    lessons: Dict[str, List[Dict]],
    session_summary: str,
    next_steps: str,
    token_budget: int = 8000
) -> TemplateLoadResult:
    """
    Convenience function to load and process a handoff template.
    
    Args:
        templates_dir: Path to templates directory
        project: Project name
        artifacts: List of artifact dictionaries
        lessons: Dictionary with "project" and "global" lesson lists
        session_summary: Summary of what was accomplished
        next_steps: What to do next session
        token_budget: Maximum tokens for output
        
    Returns:
        TemplateLoadResult with processed handoff
    """
    loader = TemplateLoader(templates_dir)
    
    variables = {
        "project": project,
        "timestamp": "TIMESTAMP",  # Will be replaced by CLI
        "artifacts": artifacts,
        "lessons_project": lessons.get("project", []),
        "lessons_global": lessons.get("global", []),
        "session_summary": session_summary,
        "next_steps": next_steps
    }
    
    return loader.load_and_process(
        template_path="core/handoff.json",
        variables=variables,
        token_budget=token_budget,
        truncate_keys=["artifacts", "lessons_project", "lessons_global"]
    )
