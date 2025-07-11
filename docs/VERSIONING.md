# Semantic Versioning and Releases

llmcpp follows [Semantic Versioning 2.0.0](https://semver.org/) for all releases.

## Version Format

```
MAJOR.MINOR.PATCH[-PRERELEASE]
```

- **MAJOR**: Incompatible API changes
- **MINOR**: Backwards-compatible functionality additions
- **PATCH**: Backwards-compatible bug fixes
- **PRERELEASE**: Optional pre-release identifier (alpha, beta, rc)

## Creating Releases

### Automated Release Script

The easiest way to create a release is using the automated script:

```bash
# Bump patch version (1.0.0 -> 1.0.1)
./scripts/release.sh patch

# Bump minor version (1.0.0 -> 1.1.0)
./scripts/release.sh minor

# Bump major version (1.0.0 -> 2.0.0)
./scripts/release.sh major

# Create prerelease (1.0.0 -> 1.0.1-alpha.1)
./scripts/release.sh prerelease

# Set specific version
./scripts/release.sh custom 2.1.0
```

The script will:
1. Validate the current git status
2. Update version numbers in all relevant files
3. Create a git commit with the version change
4. Create and push a git tag
5. Trigger GitHub Actions to build and create the release

### Manual Process

If you prefer manual control:

1. **Update version in files:**
   ```bash
   # Update CMakeLists.txt
   sed -i 's/project(llmcpp VERSION [0-9.]*/project(llmcpp VERSION 1.2.0/' CMakeLists.txt

   # Update vcpkg.json
   sed -i 's/"version": "[0-9.]*"/"version": "1.2.0"/' vcpkg.json
   ```

2. **Commit and tag:**
   ```bash
   git add CMakeLists.txt vcpkg.json
   git commit -m "chore: bump version to 1.2.0"
   git tag v1.2.0
   git push origin main
   git push origin v1.2.0
   ```

3. **GitHub Actions will automatically:**
   - Build for Linux, macOS, and Windows
   - Run tests (excluding integration tests)
   - Create release packages
   - Generate changelog from commit messages
   - Create GitHub release with assets

## Version Management

### Single Source of Truth

The version is defined in `CMakeLists.txt`:
```cmake
project(llmcpp VERSION 1.0.0 LANGUAGES CXX)
```

This version is automatically propagated to:
- Generated `llmcpp_version.h` header
- `vcpkg.json` package file
- CPack configuration
- CMake package config files

### Runtime Version Access

```cpp
#include <llmcpp.h>

// Access version information
std::cout << "Version: " << llmcpp::Version::string << std::endl;
std::cout << "Major: " << llmcpp::Version::major << std::endl;
std::cout << "Minor: " << llmcpp::Version::minor << std::endl;
std::cout << "Patch: " << llmcpp::Version::patch << std::endl;

// Compatibility checks
if (llmcpp::Version::isCompatible(1, 0)) {
    std::cout << "Compatible with v1.0+" << std::endl;
}

if (llmcpp::Version::isAtLeast(1, 2, 0)) {
    std::cout << "Has features from v1.2.0+" << std::endl;
}

// Legacy compatibility
std::cout << "Legacy: " << llmcpp::VERSION << std::endl;
```

## Release Workflow

### 1. Development
- Work on feature branches
- Create pull requests
- Merge to `main` branch

### 2. Release Preparation
- Ensure all tests pass
- Update documentation if needed
- Choose appropriate version bump

### 3. Version Bump
- Use `./scripts/release.sh <type>` or manual process
- Script validates git status and version format

### 4. Automated Build & Release
- GitHub Actions triggers on `v*.*.*` tags
- Builds on multiple platforms
- Runs comprehensive test suite
- Creates release packages
- Generates changelog
- Publishes GitHub release

### 5. Release Artifacts

Each release includes:
- **Source code** (automatically generated by GitHub)
- **Linux binary package** (`llmcpp-VERSION-Linux-x86_64.tar.gz`)
- **macOS binary package** (`llmcpp-VERSION-Darwin-arm64.tar.gz`)
- **Windows binary package** (`llmcpp-VERSION-Windows-AMD64.zip`)
- **Changelog** (generated from commit messages)

## Prerelease Versions

For testing and early access:

```bash
# Create alpha release
./scripts/release.sh prerelease
# Result: 1.0.1-alpha.1

# Custom prerelease
./scripts/release.sh custom 2.0.0-beta.1
```

Prereleases are marked as "prerelease" on GitHub and won't trigger package manager updates.

## Best Practices

### Commit Messages
Use conventional commits for better changelog generation:
```
feat: add new OpenAI model support
fix: resolve memory leak in HTTP client
docs: update API documentation
chore: bump version to 1.2.0
```

### Version Bumping Guidelines

**PATCH (1.0.0 → 1.0.1):**
- Bug fixes
- Performance improvements
- Documentation updates
- Internal refactoring

**MINOR (1.0.0 → 1.1.0):**
- New features
- New API methods (backwards compatible)
- New model support
- Optional parameters

**MAJOR (1.0.0 → 2.0.0):**
- Breaking API changes
- Removed functionality
- Changed method signatures
- Incompatible behavior changes

### Testing Before Release

```bash
# Build and test locally
cmake -B build -DLLMCPP_BUILD_TESTS=ON
cmake --build build
cd build && ctest --exclude-regex "integration"

# Test integration (requires API key)
export OPENAI_API_KEY="your-key"
export LLMCPP_RUN_INTEGRATION_TESTS=1
ctest --tests-regex "integration"
```

## Troubleshooting

### Version Mismatch Error
If you get a version mismatch error during release creation:
```bash
# Check current version
grep "project(llmcpp VERSION" CMakeLists.txt

# Update to match your desired tag
sed -i 's/project(llmcpp VERSION [0-9.]*/project(llmcpp VERSION 1.2.0/' CMakeLists.txt
```

### Failed Release Build
Monitor the GitHub Actions at:
`https://github.com/lucaromagnoli/llmcpp/actions`

Common issues:
- Test failures on specific platforms
- Missing dependencies
- Version format validation errors

### Rollback a Release
```bash
# Delete tag locally and remotely
git tag -d v1.2.0
git push --delete origin v1.2.0

# Delete GitHub release (manual via web interface)
```
