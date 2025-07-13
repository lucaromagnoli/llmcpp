#!/bin/bash

# Release script for llmcpp
# Helps with semantic versioning and creating releases

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 <version_type> [version]"
    echo ""
    echo "Version types:"
    echo "  major       - Bump major version (1.0.0 -> 2.0.0)"
    echo "  minor       - Bump minor version (1.0.0 -> 1.1.0)"
    echo "  patch       - Bump patch version (1.0.0 -> 1.0.1)"
    echo "  prerelease  - Add prerelease suffix (1.0.0 -> 1.0.1-alpha.1)"
    echo "  custom      - Set specific version (requires version argument)"
    echo ""
    echo "Examples:"
    echo "  $0 patch                 # 1.0.0 -> 1.0.1"
    echo "  $0 minor                 # 1.0.0 -> 1.1.0"
    echo "  $0 major                 # 1.0.0 -> 2.0.0"
    echo "  $0 prerelease            # 1.0.0 -> 1.0.1-alpha.1"
    echo "  $0 custom 2.1.0          # Set to specific version"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

get_current_version() {
    grep "project(llmcpp VERSION" "$PROJECT_DIR/CMakeLists.txt" | sed 's/.*VERSION \([0-9.]*\).*/\1/'
}

update_version_in_files() {
    local new_version="$1"
    
    print_info "Updating version to $new_version in project files..."
    
    # Update CMakeLists.txt
    sed -i.bak "s/project(llmcpp VERSION [0-9.]*/project(llmcpp VERSION $new_version/" "$PROJECT_DIR/CMakeLists.txt"
    rm "$PROJECT_DIR/CMakeLists.txt.bak"
    
    # Update vcpkg.json
    if [ -f "$PROJECT_DIR/vcpkg.json" ]; then
        sed -i.bak "s/\"version\": \"[0-9.]*\"/\"version\": \"$new_version\"/" "$PROJECT_DIR/vcpkg.json"
        rm "$PROJECT_DIR/vcpkg.json.bak"
    fi
    
    print_success "Updated version in project files"
}

update_changelog() {
    local new_version="$1"
    
    print_info "Updating changelog for version $new_version..."
    
    # Run the changelog update script
    if [ -f "$PROJECT_DIR/scripts/update-changelog.py" ]; then
        cd "$PROJECT_DIR"
        
        # First, update with new commits
        python3 scripts/update-changelog.py update
        
        # Then replace [Unreleased] with the actual version
        python3 scripts/update-changelog.py replace "$new_version"
        
        # Check if changelog was updated
        if git diff --quiet CHANGELOG.md; then
            print_warning "No changes detected in changelog"
        else
            print_success "Changelog updated successfully"
        fi
    else
        print_warning "Changelog update script not found, skipping changelog update"
    fi
}

validate_version() {
    local version="$1"
    if ! [[ $version =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.-]+)?$ ]]; then
        print_error "Invalid version format: $version"
        print_error "Expected format: MAJOR.MINOR.PATCH or MAJOR.MINOR.PATCH-prerelease"
        exit 1
    fi
}

increment_version() {
    local current_version="$1"
    local bump_type="$2"
    
    IFS='.' read -r major minor patch <<< "$current_version"
    
    case $bump_type in
        major)
            major=$((major + 1))
            minor=0
            patch=0
            ;;
        minor)
            minor=$((minor + 1))
            patch=0
            ;;
        patch)
            patch=$((patch + 1))
            ;;
        prerelease)
            patch=$((patch + 1))
            echo "${major}.${minor}.${patch}-alpha.1"
            return
            ;;
        *)
            print_error "Unknown bump type: $bump_type"
            exit 1
            ;;
    esac
    
    echo "${major}.${minor}.${patch}"
}

check_git_status() {
    if ! git diff-index --quiet HEAD --; then
        print_warning "You have uncommitted changes. Please commit or stash them first."
        git status --porcelain
        exit 1
    fi
}

main() {
    cd "$PROJECT_DIR"
    
    if [ $# -lt 1 ]; then
        print_error "Missing version type argument"
        print_usage
        exit 1
    fi
    
    local version_type="$1"
    local custom_version="$2"
    
    # Check git status
    check_git_status
    
    # Get current version
    local current_version
    current_version=$(get_current_version)
    print_info "Current version: $current_version"
    
    # Calculate new version
    local new_version
    if [ "$version_type" = "custom" ]; then
        if [ -z "$custom_version" ]; then
            print_error "Custom version requires version argument"
            print_usage
            exit 1
        fi
        new_version="$custom_version"
    else
        new_version=$(increment_version "$current_version" "$version_type")
    fi
    
    # Validate new version
    validate_version "$new_version"
    print_info "New version: $new_version"
    
    # Show what will happen
    echo ""
    print_info "This will:"
    echo "  1. Update version in CMakeLists.txt and vcpkg.json"
    echo "  2. Update CHANGELOG.md with new commits since last release"
    echo "  3. Create a git commit with all version and changelog changes"
    echo "  4. Create and push a git tag v$new_version"
    echo "  5. Push the current branch to origin"
    echo "  6. Trigger GitHub Actions to create a release (if on main branch)"
    echo ""
    
    # Update version in files
    update_version_in_files "$new_version"
    
    # Update changelog
    update_changelog "$new_version"
    
    # Create git commit with all changes
    print_info "Creating git commit with version and changelog changes..."
    git add CMakeLists.txt vcpkg.json CHANGELOG.md
    git commit -m "chore: bump version to $new_version and update changelog"
    
    # Create and push tag
    print_info "Creating and pushing tag v$new_version..."
    git tag "v$new_version"
    
    # Get current branch name
    local current_branch
    current_branch=$(git branch --show-current)
    print_info "Pushing current branch: $current_branch"
    git push origin "$current_branch"
    git push origin "v$new_version"
    
    print_success "Release v$new_version created!"
    print_info "GitHub Actions will now build and create the release."
    print_info "You can monitor progress at: https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\([^.]*\).*/\1/')/actions"
}

# Run main function
main "$@" 