#!/usr/bin/env python3
"""
Automated changelog generator for llmcpp
Parses conventional commits and updates CHANGELOG.md
"""

import os
import re
import sys
import subprocess
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple


class ChangelogGenerator:
    def __init__(self):
        self.changelog_file = "CHANGELOG.md"
        self.project_name = "llmcpp"
        
        # Conventional commit types and their changelog categories
        self.commit_categories = {
            'feat': 'Added',
            'fix': 'Fixed', 
            'docs': 'Documentation',
            'style': 'Changed',
            'refactor': 'Changed',
            'perf': 'Performance',
            'test': 'Testing',
            'build': 'Build',
            'ci': 'Build',
            'chore': 'Maintenance',
            'revert': 'Reverted'
        }
    
    def get_current_version(self) -> str:
        """Extract current version from CMakeLists.txt"""
        cmake_file = Path("CMakeLists.txt")
        if not cmake_file.exists():
            return "unknown"
        
        with open(cmake_file, 'r') as f:
            content = f.read()
            match = re.search(r'project\(llmcpp VERSION ([0-9.]+)\)', content)
            return match.group(1) if match else "unknown"
    
    def get_last_tag(self) -> str:
        """Get the last git tag or initial commit"""
        try:
            result = subprocess.run(
                ['git', 'describe', '--tags', '--abbrev=0'],
                capture_output=True, text=True, check=False
            )
            if result.returncode == 0:
                return result.stdout.strip()
        except Exception:
            pass
        
        # If no tags exist, use the initial commit
        try:
            result = subprocess.run(
                ['git', 'rev-list', '--max-parents=0', 'HEAD'],
                capture_output=True, text=True, check=True
            )
            return result.stdout.strip()
        except Exception:
            return "HEAD~10"  # Fallback
    
    def parse_conventional_commit(self, commit_msg: str) -> Optional[Tuple[str, str]]:
        """Parse conventional commit and return (type, description)"""
        # Pattern: type(scope): description
        pattern = r'^(feat|fix|docs|style|refactor|perf|test|chore|build|ci|revert)(?:\([^)]+\))?: (.+)$'
        match = re.match(pattern, commit_msg)
        
        if match:
            commit_type = match.group(1)
            description = match.group(2)
            return commit_type, description
        return None
    
    def get_commits_since_tag(self, last_tag: str) -> List[Tuple[str, str]]:
        """Get commits since last tag"""
        try:
            result = subprocess.run(
                ['git', 'log', '--pretty=format:%H %s', f'{last_tag}..HEAD', '--reverse'],
                capture_output=True, text=True, check=True
            )
            
            commits = []
            for line in result.stdout.strip().split('\n'):
                if line.strip():
                    parts = line.split(' ', 1)
                    if len(parts) == 2:
                        commits.append((parts[0], parts[1]))
            return commits
        except Exception as e:
            print(f"Warning: Could not get commits: {e}")
            return []
    
    def categorize_commits(self, commits: List[Tuple[str, str]]) -> Dict[str, List[str]]:
        """Categorize commits by type"""
        categories = {
            'Added': [],
            'Changed': [],
            'Fixed': [],
            'Documentation': [],
            'Performance': [],
            'Testing': [],
            'Build': [],
            'Maintenance': [],
            'Reverted': [],
            'Other': []
        }
        
        for commit_hash, commit_msg in commits:
            parsed = self.parse_conventional_commit(commit_msg)
            if parsed:
                commit_type, description = parsed
                category = self.commit_categories.get(commit_type, 'Other')
                categories[category].append(description)
            else:
                # Non-conventional commit
                categories['Other'].append(commit_msg)
        
        return categories
    
    def generate_changelog_content(self, categories: Dict[str, List[str]]) -> str:
        """Generate changelog content from categorized commits"""
        if not any(categories.values()):
            return ""
        
        content = f"## [Unreleased] - {datetime.now().strftime('%Y-%m-%d')}\n\n"
        
        # Add sections with content
        for category, entries in categories.items():
            if entries:
                content += f"### {category}\n"
                for entry in entries:
                    content += f"- {entry}\n"
                content += "\n"
        
        return content
    
    def update_changelog(self) -> None:
        """Update the changelog file"""
        current_version = self.get_current_version()
        last_tag = self.get_last_tag()
        
        print(f"Updating changelog for version {current_version}")
        print(f"Generating changelog from {last_tag} to current HEAD...")
        
        # Get and categorize commits
        commits = self.get_commits_since_tag(last_tag)
        if not commits:
            print("No new commits since last tag")
            return
        
        categories = self.categorize_commits(commits)
        new_content = self.generate_changelog_content(categories)
        
        if not new_content:
            print("No conventional commits found")
            return
        
        # Update or create changelog file
        if Path(self.changelog_file).exists():
            # Insert new content after header
            with open(self.changelog_file, 'r') as f:
                lines = f.readlines()
            
            # Find where to insert (after header)
            insert_pos = 0
            for i, line in enumerate(lines):
                if line.startswith('# Changelog'):
                    insert_pos = i + 1
                    break
            
            # Insert new content
            new_lines = lines[:insert_pos] + ['\n'] + [new_content] + ['\n'] + lines[insert_pos:]
            
            with open(self.changelog_file, 'w') as f:
                f.writelines(new_lines)
        else:
            # Create new changelog file
            header = """# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

"""
            with open(self.changelog_file, 'w') as f:
                f.write(header + new_content)
        
        print("✅ Changelog updated successfully!")
    
    def generate_release_notes(self, prev_tag: str, current_tag: str) -> str:
        """Generate release notes for GitHub release"""
        commits = self.get_commits_since_tag(prev_tag)
        if not commits:
            return "No changes in this release."
        
        categories = self.categorize_commits(commits)
        
        # Generate release notes format
        release_notes = "## 🚀 What's Changed\n\n"
        
        # Add sections with content
        for category, entries in categories.items():
            if entries:
                release_notes += f"### {category}\n"
                for entry in entries:
                    release_notes += f"- {entry}\n"
                release_notes += "\n"
        
        # Add full changelog link
        release_notes += f"**Full Changelog**: https://github.com/lucaromagnoli/llmcpp/compare/{prev_tag}...{current_tag}\n"
        
        return release_notes
    
    def show_version(self) -> None:
        """Show current version"""
        version = self.get_current_version()
        print(f"Current version: {version}")
    
    def show_help(self) -> None:
        """Show help message"""
        print("""Usage: python scripts/update-changelog.py [command]

Commands:
  update         - Update changelog with new commits (default)
  init           - Initialize changelog file
  release-notes  - Generate release notes for GitHub release
  version        - Show current version
  help           - Show this help message

Examples:
  python scripts/update-changelog.py update
  python scripts/update-changelog.py release-notes v1.0.3 v1.0.4
""")


def main():
    generator = ChangelogGenerator()
    
    command = sys.argv[1] if len(sys.argv) > 1 else "update"
    
    if command == "update":
        generator.update_changelog()
    elif command == "init":
        if not Path(generator.changelog_file).exists():
            generator.update_changelog()
        else:
            print("⚠️  Changelog already exists. Use 'update' to add new entries.")
    elif command == "release-notes":
        if len(sys.argv) < 4:
            print("❌ Usage: python scripts/update-changelog.py release-notes <prev_tag> <current_tag>")
            sys.exit(1)
        prev_tag = sys.argv[2]
        current_tag = sys.argv[3]
        release_notes = generator.generate_release_notes(prev_tag, current_tag)
        print(release_notes)
    elif command == "version":
        generator.show_version()
    elif command in ["help", "-h", "--help"]:
        generator.show_help()
    else:
        print(f"❌ Unknown command: {command}")
        print("Use 'python scripts/update-changelog.py help' for usage information")
        sys.exit(1)


if __name__ == "__main__":
    main() 