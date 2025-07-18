# Changelog

## [Unreleased] - Unreleased



## [1.0.19] - 2025-07-13

### Added
- skip release build-and-test if CI has already passed for the commit

### Fixed
- simplify CI condition to prevent tag builds

### Other
- Merge pull request #30 from lucaromagnoli/feat/add-models
- Remove o3 model integration test; only test o3-mini (no temperature or maxTokens)
- Commit remaining changes: core, openai, and unit test updates for optional params and o3-mini logic
- Fix maxTokens test: now optional instead of defaulting to 200
- Bump version to 1.0.18 for patch release

## [1.0.17] - 2025-07-13

### Fixed
- add explicit condition to skip CI on tag pushes

## [1.0.16] - 2025-07-13

### Fixed
- add checkout step to check-open-prs job in release workflow

## [1.0.15] - 2025-07-13

### Build
- do not run CI workflow on version tags (only release workflow runs on tags)
- optimize workflows - merge code quality into CI and skip release on open PRs

## [1.0.14] - 2025-07-13

### Fixed
- correct GPT-4.5 model status - it's a current preview model, not deprecated
- update tests to use correct GPT-4.5 model string (gpt-4.5-preview)
- implement parameter filtering for reasoning models (O-series)

### Documentation
- update README to clarify model enum usage and remove recommendation references

### Maintenance
- remove duplicate release-workflow.sh script

### Other
- Merge pull request #29 from lucaromagnoli/feat/release-test
- Remove getRecommendedModel and related tests, clean up OpenAI model enum logic

## [1.0.13] - 2025-07-13

### Changed
- remove macOS from release builds for cost optimization

## [1.0.12] - 2025-07-13

### Added
- use Python script for robust release notes extraction

### Fixed
- resolve awk syntax error in release notes generation

## [1.0.11] - 2025-07-13

### Fixed
- improve release notes generation and changelog handling

### Maintenance
- bump version to 1.0.10 and update changelog

### Other
- Merge pull request #27 from lucaromagnoli/feat/release_process
- Merge pull request #28 from lucaromagnoli/fix/pipeline-merge

## [1.0.10] - 2025-07-13

### Other
- Merge pull request #27 from lucaromagnoli/feat/release_process



## [1.0.11] - 2025-07-13

### Fixed
- improve release notes generation and changelog handling

### Maintenance
- bump version to 1.0.10 and update changelog

### Other
- Merge pull request #27 from lucaromagnoli/feat/release_process
- Merge pull request #28 from lucaromagnoli/fix/pipeline-merge

## [1.0.9] - 2025-07-13

### Added
- add interactive release workflow with changelog editing

### Other
- Merge pull request #26 from lucaromagnoli/feat/cost-opt



## [1.0.11] - 2025-07-13

### Fixed
- improve release notes generation and changelog handling

### Maintenance
- bump version to 1.0.10 and update changelog

### Other
- Merge pull request #27 from lucaromagnoli/feat/release_process
- Merge pull request #28 from lucaromagnoli/fix/pipeline-merge

## [1.0.8] - 2025-07-13

### Added
- optimize GitHub Actions costs by removing macOS from regular CI
- streamline release process to eliminate double PR workflow

### Other
- Merge pull request #23 from lucaromagnoli/improve-async
- Merge branch 'main' into release/v1.0.7
- Merge pull request #24 from lucaromagnoli/release/v1.0.7
- Merge pull request #25 from lucaromagnoli/cost-optimise-pipeline



All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.8] - 2025-07-13

### Added
- add type-safe Model enum support with convenience methods
- modernize CMake for proper install and export

### Other
- Merge pull request #17 from lucaromagnoli/fix/llmrequest
