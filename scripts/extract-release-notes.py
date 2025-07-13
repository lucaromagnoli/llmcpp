#!/usr/bin/env python3
import sys
import re

if len(sys.argv) < 2:
    print("Usage: extract-release-notes.py <version>")
    sys.exit(1)

version = sys.argv[1]
changelog = "CHANGELOG.md"
repo_url = "https://github.com/lucaromagnoli/llmcpp"

with open(changelog, "r") as f:
    content = f.read()

pattern = rf"^## \[{re.escape(version)}\][^\n]*\n(.*?)(?=^## \[|\Z)"
match = re.search(pattern, content, re.MULTILINE | re.DOTALL)
if match:
    notes = match.group(1).strip()
    if notes:
        print(f"## 🚀 What's Changed\n\n{notes}")
    else:
        print(f"## 🚀 Release {version}\n\nNo detailed changes found for this version.\n\n**Full Changelog**: {repo_url}/releases/tag/v{version}")
else:
    print(f"## 🚀 Release {version}\n\nThis release includes various improvements and bug fixes.\n\n**Full Changelog**: {repo_url}/releases/tag/v{version}") 