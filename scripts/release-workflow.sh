#!/bin/bash

# Release workflow for llmcpp
# Creates a new version, updates changelog, and creates a PR

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="llmcpp"
RELEASE_BRANCH_PREFIX="release"

# Get current version from CMakeLists.txt
get_current_version() {
    grep -o 'project(llmcpp VERSION [0-9.]*)' CMakeLists.txt | sed 's/.*VERSION \([0-9.]*\).*/\1/'
}

# Update version in files
update_version() {
    local new_version=$1
    local current_version=$(get_current_version)
    
    echo -e "${BLUE}Updating version from $current_version to $new_version${NC}"
    
    # Update CMakeLists.txt
    sed -i.bak "s/project(llmcpp VERSION [0-9.]*)/project(llmcpp VERSION $new_version)/" CMakeLists.txt
    rm CMakeLists.txt.bak
    
    # Update vcpkg.json if it exists
    if [ -f "vcpkg.json" ]; then
        sed -i.bak "s/\"version\": \"[0-9.]*\"/\"version\": \"$new_version\"/" vcpkg.json
        rm vcpkg.json.bak
    fi
    
    echo -e "${GREEN}Version updated to $new_version${NC}"
}

# Create release branch
create_release_branch() {
    local version=$1
    local branch_name="${RELEASE_BRANCH_PREFIX}/v${version}"
    
    echo -e "${BLUE}Creating release branch: $branch_name${NC}"
    
    # Ensure we're on main and up to date
    git checkout main
    git pull origin main
    
    # Create and checkout release branch
    git checkout -b "$branch_name"
    
    echo -e "${GREEN}Release branch created: $branch_name${NC}"
}

# Update changelog
update_changelog() {
    echo -e "${BLUE}Updating changelog...${NC}"
    
    # Run the changelog script
    python scripts/update-changelog.py update
    
    echo -e "${GREEN}Changelog updated${NC}"
}

# Commit changes
commit_changes() {
    local version=$1
    
    echo -e "${BLUE}Committing changes...${NC}"
    
    git add .
    git commit -m "chore: prepare release v${version}

- Bump version to ${version}
- Update changelog with latest changes"
    
    echo -e "${GREEN}Changes committed${NC}"
}

# Push branch and create PR
create_pr() {
    local version=$1
    local branch_name="${RELEASE_BRANCH_PREFIX}/v${version}"
    
    echo -e "${BLUE}Pushing branch and creating PR...${NC}"
    
    # Push the branch
    git push origin "$branch_name"
    
    # Create PR using GitHub CLI if available
    if command -v gh &> /dev/null; then
        echo -e "${BLUE}Creating PR with GitHub CLI...${NC}"
        gh pr create \
            --title "Release v${version}" \
            --body "## Release v${version}

This PR contains the release preparation for version ${version}.

### Changes:
- Bump version to ${version}
- Update changelog with latest changes

### Checklist:
- [ ] All tests pass
- [ ] Changelog is complete
- [ ] Version numbers are correct
- [ ] Ready for review and merge

Once merged, this will trigger the release workflow." \
            --base main \
            --head "$branch_name"
    else
        echo -e "${YELLOW}GitHub CLI not found. Please create PR manually:${NC}"
        echo -e "${BLUE}https://github.com/lucaromagnoli/llmcpp/compare/main...$branch_name${NC}"
    fi
    
    echo -e "${GREEN}Release branch pushed and PR created${NC}"
}

# Main release workflow
main() {
    if [ $# -eq 0 ]; then
        echo -e "${RED}Usage: $0 <version>${NC}"
        echo ""
        echo "Examples:"
        echo "  $0 1.0.4    # Patch release"
        echo "  $0 1.1.0    # Minor release"
        echo "  $0 2.0.0    # Major release"
        exit 1
    fi
    
    local new_version=$1
    local current_version=$(get_current_version)
    
    echo -e "${BLUE}Starting release workflow for v${new_version}${NC}"
    echo -e "${BLUE}Current version: ${current_version}${NC}"
    echo ""
    
    # Validate version format
    if [[ ! $new_version =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo -e "${RED}Invalid version format. Use semantic versioning (e.g., 1.0.4)${NC}"
        exit 1
    fi
    
    # Check if version is newer
    if [ "$new_version" = "$current_version" ]; then
        echo -e "${RED}Version $new_version is already the current version${NC}"
        exit 1
    fi
    
    # Check git status
    if [ -n "$(git status --porcelain)" ]; then
        echo -e "${RED}Working directory is not clean. Please commit or stash changes first.${NC}"
        git status --short
        exit 1
    fi
    
    # Execute release workflow
    create_release_branch "$new_version"
    update_version "$new_version"
    update_changelog
    commit_changes "$new_version"
    create_pr "$new_version"
    
    echo ""
    echo -e "${GREEN}✅ Release workflow completed!${NC}"
    echo -e "${BLUE}Next steps:${NC}"
    echo "1. Review the PR"
    echo "2. Run tests: make test-ci"
    echo "3. Merge the PR when ready"
    echo "4. The release will be created automatically"
}

# Run main function
main "$@" 