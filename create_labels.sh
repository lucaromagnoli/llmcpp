#!/bin/bash

# Script to create GitHub labels for llmcpp repository
# Usage: ./create_labels.sh
# Prerequisites: GitHub CLI (gh) must be installed and authenticated

set -e

REPO="lucaromagnoli/llmcpp"

echo "Creating GitHub labels for llmcpp repository..."
echo "Repository: $REPO"
echo ""

# Check if gh CLI is available
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is not installed."
    echo "Please install it from: https://cli.github.com/"
    exit 1
fi

# Verify authentication
if ! gh auth status &> /dev/null; then
    echo "Error: Not authenticated with GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

echo "Creating labels..."

# Core implementation labels
gh label create "core" --color "d73a4a" --description "Core library functionality" || true
gh label create "high-priority" --color "b60205" --description "High priority tasks" || true
gh label create "medium-priority" --color "fbca04" --description "Medium priority tasks" || true
gh label create "low-priority" --color "0e8a16" --description "Low priority tasks" || true

# Feature area labels
gh label create "streaming" --color "1d76db" --description "Streaming functionality" || true
gh label create "security" --color "5319e7" --description "Security and authentication" || true
gh label create "provider" --color "0052cc" --description "LLM provider implementations" || true
gh label create "packaging" --color "c2e0c6" --description "Package management and distribution" || true
gh label create "performance" --color "f9d0c4" --description "Performance optimization" || true
gh label create "optimization" --color "f9d0c4" --description "Code optimization" || true

# Documentation and examples
gh label create "documentation" --color "0075ca" --description "Documentation improvements" || true
gh label create "examples" --color "7057ff" --description "Example code and tutorials" || true

# Testing labels
gh label create "testing" --color "c5def5" --description "Testing infrastructure" || true
gh label create "integration" --color "bfd4f2" --description "Integration tests" || true

# Distribution labels
gh label create "distribution" --color "c2e0c6" --description "Distribution and deployment" || true

# Update enhancement label color to match our scheme
gh label edit "enhancement" --color "a2eeef" --description "New feature or request" || true

echo ""
echo "✅ GitHub labels created successfully!"
echo ""
echo "Labels created:"
echo "  Priority: high-priority (red), medium-priority (yellow), low-priority (green)"
echo "  Areas: core, streaming, security, provider, packaging, performance"
echo "  Types: documentation, examples, testing, integration, distribution"
echo ""
echo "You can view labels at: https://github.com/$REPO/labels" 