# Release Process

This document outlines the standardized release process for llmcpp to ensure safe, reproducible, and reviewed releases.

## Overview

The release process follows a structured workflow to guarantee that:
- All code in a release is properly reviewed and merged to `main`
- Version bumps are tracked and reviewed
- Releases are always built from the `main` branch
- Release artifacts are consistent and reproducible

## Prerequisites

- All feature work is complete and merged to `main`
- CI is passing on `main`
- You have the necessary permissions to create tags and releases

## Release Workflow

### 1. Finish and Merge Features

Ensure all features for this release are complete:
- Develop features on separate branches
- Open PRs to `main` and merge after review
- Verify CI passes on `main`

### 2. Create a Release Branch

From `main`, create a new branch for the release:

```bash
git checkout main
git pull origin main
git checkout -b release/v1.0.4  # Replace with your version
```

### 3. Bump the Version

On the release branch, use the release script to bump the version:

```bash
# For patch releases (1.0.3 -> 1.0.4)
./scripts/release.sh patch

# For minor releases (1.0.3 -> 1.1.0)
./scripts/release.sh minor

# For major releases (1.0.3 -> 2.0.0)
./scripts/release.sh major

# For custom versions
./scripts/release.sh custom 1.0.4
```

The script will:
- Update version in `CMakeLists.txt` and `vcpkg.json`
- Create a commit with the version changes
- Push the branch to origin
- Create and push a tag (optional - you can cancel this)

### 4. Open a Version Bump PR

Create a Pull Request from your release branch to `main`:
- Ensure the PR only contains version bump changes
- Add a description explaining what's being released
- Wait for CI to pass

### 5. Merge the Version Bump PR

After review and CI pass:
- Merge the PR into `main`
- Delete the release branch

### 6. Tag the Release

On `main`, create and push the release tag:

```bash
git checkout main
git pull origin main
git tag v1.0.4  # Replace with your version
git push origin v1.0.4
```

This triggers the release pipeline which:
- Builds and tests on all platforms (Windows, macOS, Linux)
- Generates changelog and release notes
- Creates release artifacts (.tar.gz, .zip)
- Publishes to GitHub Releases

## Version Bumping Guidelines

### Semantic Versioning

Follow [Semantic Versioning](https://semver.org/):

- **MAJOR** version for incompatible API changes
- **MINOR** version for backwards-compatible functionality additions
- **PATCH** version for backwards-compatible bug fixes

### Pre-release Versions

For testing releases, use pre-release suffixes:
- `1.0.4-rc1` (Release Candidate)
- `1.0.4-alpha.1` (Alpha)
- `1.0.4-beta.1` (Beta)

## Important Notes

### Never Tag from Feature Branches

- **Always create release tags from `main`**
- The release pipeline will fail if the tag is not on `main`
- This ensures releases always match the main branch code

### Version Validation

The release pipeline includes version validation:
- Tag version must be newer than the current version in `CMakeLists.txt`
- This prevents accidental releases of older versions
- Use the release script to properly bump versions

### Release Artifacts

Each release includes:
- Source code archives
- Pre-built binaries for all supported platforms
- Generated changelog and release notes
- CMake package configuration files

## Troubleshooting

### Release Pipeline Fails

If the release pipeline fails:

1. **Check the logs** for specific error messages
2. **Verify version consistency** between tag and `CMakeLists.txt`
3. **Ensure the tag is on `main`**
4. **Check that all dependencies are available**

### Version Mismatch

If you get a version mismatch error:
- Ensure you used the release script to bump the version
- Verify the tag version is newer than the current version
- Check that the version bump PR was merged to `main`

### Manual Release

If you need to create a release manually:
1. Follow the same workflow steps
2. Ensure all version files are updated
3. Create the tag from `main`
4. Monitor the release pipeline

## Automation

The release process is partially automated:
- `scripts/release.sh` - Version bumping and tagging
- `scripts/update-changelog.py` - Changelog generation
- GitHub Actions - Build, test, and package releases

Future improvements may include:
- Automated PR creation for version bumps
- Automated release notes generation
- Integration with issue tracking systems

## Support

If you encounter issues with the release process:
1. Check this documentation
2. Review recent successful releases
3. Check the GitHub Actions logs
4. Open an issue for persistent problems
