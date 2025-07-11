#!/bin/bash

# Script to create GitHub milestones for llmcpp repository
# Usage: ./create_milestones.sh
# Prerequisites: GitHub CLI (gh) must be installed and authenticated

set -e

REPO="lucaromagnoli/llmcpp"

echo "Creating GitHub milestones for llmcpp repository..."
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

echo "Creating milestones..."

# Create milestones with descriptions and due dates
gh api \
  --method POST \
  -H "Accept: application/vnd.github+json" \
  "/repos/$REPO/milestones" \
  -f title="v1.0.0" \
  -f description="Initial release with core OpenAI client functionality, streaming support, and basic examples" \
  -f state="open" \
  -f due_on="2024-12-31T23:59:59Z"

gh api \
  --method POST \
  -H "Accept: application/vnd.github+json" \
  "/repos/$REPO/milestones" \
  -f title="v1.1.0" \
  -f description="Enhanced testing and integration test suite" \
  -f state="open" \
  -f due_on="2025-01-31T23:59:59Z"

gh api \
  --method POST \
  -H "Accept: application/vnd.github+json" \
  "/repos/$REPO/milestones" \
  -f title="v1.2.0" \
  -f description="Package manager integration and distribution improvements" \
  -f state="open" \
  -f due_on="2025-02-28T23:59:59Z"

gh api \
  --method POST \
  -H "Accept: application/vnd.github+json" \
  "/repos/$REPO/milestones" \
  -f title="v1.3.0" \
  -f description="Performance optimization and benchmarking" \
  -f state="open" \
  -f due_on="2025-03-31T23:59:59Z"

gh api \
  --method POST \
  -H "Accept: application/vnd.github+json" \
  "/repos/$REPO/milestones" \
  -f title="v2.0.0" \
  -f description="Multi-provider support with Anthropic Claude integration" \
  -f state="open" \
  -f due_on="2025-06-30T23:59:59Z"

echo ""
echo "✅ GitHub milestones created successfully!"
echo ""
echo "Milestones created:"
echo "  v1.0.0 - Core functionality (Dec 2024)"
echo "  v1.1.0 - Enhanced testing (Jan 2025)"
echo "  v1.2.0 - Package management (Feb 2025)"
echo "  v1.3.0 - Performance optimization (Mar 2025)"
echo "  v2.0.0 - Multi-provider support (Jun 2025)"
echo ""
echo "You can view milestones at: https://github.com/$REPO/milestones" 