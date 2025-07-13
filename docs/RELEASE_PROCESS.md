# Release Process

This document describes the streamlined release process for llmcpp that eliminates the double PR workflow issue.

## Overview

The release process has been optimized to handle all version and changelog updates **before** tagging, ensuring that:
- All changes are committed together
- No post-tag commits are needed
- The release workflow only builds and publishes
- No double PR workflow is required

## Release Process

### 1. Prepare for Release

Ensure your working directory is clean:
```bash
git status
# Should show no uncommitted changes
```

### 2. Run the Release Script

Use the release script to automate the entire process:

```bash
# For a patch release (1.0.0 -> 1.0.1)
./scripts/release.sh patch

# For a minor release (1.0.0 -> 1.1.0)
./scripts/release.sh minor

# For a major release (1.0.0 -> 2.0.0)
./scripts/release.sh major

# For a prerelease (1.0.0 -> 1.0.1-alpha.1)
./scripts/release.sh prerelease

# For a custom version
./scripts/release.sh custom 2.1.0
```

### 3. What the Release Script Does

The release script performs the following steps in order:

1. **Validates git status** - Ensures no uncommitted changes
2. **Calculates new version** - Based on the version type
3. **Updates version files** - CMakeLists.txt and vcpkg.json
4. **Updates changelog** - Adds new commits and replaces [Unreleased] with version
5. **Creates commit** - Commits all changes together
6. **Creates and pushes tag** - Tags the release and pushes to remote
7. **Triggers workflow** - GitHub Actions builds and publishes the release

### 4. GitHub Actions Workflow

The release workflow (`release.yml`) is triggered by tag pushes and:

1. **Validates the release** - Checks version format and changelog
2. **Builds on all platforms** - Linux, macOS, Windows
3. **Runs tests** - Ensures quality
4. **Creates release** - Publishes to GitHub Releases with assets

## Key Improvements

### Before (Double PR Issue)
- Release script created tag
- Workflow updated changelog after tag
- Post-tag commits created drift
- Required second PR to sync changes

### After (Streamlined)
- Release script updates everything before tagging
- Workflow only builds and publishes
- No post-tag commits
- Single, clean release process

## Validation

The release workflow includes validation steps:

- **Version validation** - Ensures tag version is newer than current
- **Changelog validation** - Ensures changelog contains the release version
- **Build validation** - Tests on all platforms

## Manual Steps (if needed)

### Update Changelog Manually
```bash
# Add new commits to changelog
python3 scripts/update-changelog.py update

# Replace [Unreleased] with specific version
python3 scripts/update-changelog.py replace 1.0.4
```

### Check Current Version
```bash
python3 scripts/update-changelog.py version
```

## Troubleshooting

### Release Script Fails
- Ensure no uncommitted changes
- Check that you're on the correct branch
- Verify version format is valid

### Workflow Fails
- Check that changelog contains the release version
- Ensure version in CMakeLists.txt matches tag
- Review build logs for platform-specific issues

### Changelog Issues
- Ensure commits follow conventional commit format
- Check that the changelog file exists and is properly formatted
- Verify the update script has proper permissions

## Best Practices

1. **Use conventional commits** - Makes changelog generation automatic
2. **Test locally first** - Run builds and tests before releasing
3. **Review changelog** - Ensure it accurately reflects changes
4. **Use appropriate version bump** - Follow semantic versioning
5. **Monitor workflow** - Check GitHub Actions for any issues

## Rollback

If a release needs to be rolled back:

1. **Delete the tag** (if not published):
   ```bash
   git tag -d v1.0.1
   git push origin :refs/tags/v1.0.1
   ```

2. **Revert the commit**:
   ```bash
   git revert <commit-hash>
   git push origin main
   ```

3. **Delete GitHub release** - Via GitHub web interface

## Future Improvements

- [ ] Add release notes preview before tagging
- [ ] Implement release candidate workflow
- [ ] Add automated dependency updates
- [ ] Include security scanning in release process
