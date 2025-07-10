#!/bin/bash

# Script to create GitHub issues for llmcpp development roadmap
# Usage: ./create_issues.sh
# Prerequisites: GitHub CLI (gh) must be installed and authenticated

set -e

REPO="lucaromagnoli/llmcpp"

echo "Creating GitHub issues for llmcpp development roadmap..."
echo "Repository: $REPO"
echo ""

# Check if gh CLI is available
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is not installed."
    echo "Please install it from: https://cli.github.com/"
    echo ""
    echo "Alternative: Copy the issue templates from ISSUES.md to GitHub manually"
    exit 1
fi

# Verify authentication
if ! gh auth status &> /dev/null; then
    echo "Error: Not authenticated with GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

echo "Creating issues..."

# Core Implementation Issues
gh issue create \
    --title "🚀 Implement OpenAI HTTP Client" \
    --body "## Description
Implement the core HTTP client for OpenAI API using cpp-httplib.

## Tasks
- [ ] Create HTTP client wrapper around cpp-httplib
- [ ] Implement authentication with Bearer tokens
- [ ] Add request/response serialization with nlohmann/json
- [ ] Handle HTTP status codes and error responses
- [ ] Add timeout and retry logic
- [ ] Support for custom headers and user agents

## Acceptance Criteria
- [ ] Can successfully make authenticated requests to OpenAI API
- [ ] Proper error handling for network issues, auth failures, rate limits
- [ ] JSON request/response serialization works correctly
- [ ] Tests cover success and error scenarios
- [ ] Documentation includes usage examples

## Dependencies
- None (foundational feature)

## Estimate
**Priority:** High  
**Effort:** 2-3 days" \
    --label "enhancement,core,high-priority" \
    --milestone "v1.0.0"

gh issue create \
    --title "🔧 Implement OpenAI Chat Completions API" \
    --body "## Description
Implement the OpenAI Chat Completions API endpoint with full parameter support.

## Tasks
- [ ] Implement chat completions request formatting
- [ ] Support all OpenAI parameters (temperature, max_tokens, etc.)
- [ ] Handle model selection and validation
- [ ] Parse chat completions responses
- [ ] Extract usage metadata (token counts)
- [ ] Support function calling / tools
- [ ] Add conversation context management

## Acceptance Criteria
- [ ] Can send chat completion requests with custom parameters
- [ ] Supports all major OpenAI models (GPT-4, GPT-3.5, etc.)
- [ ] Function calling works correctly
- [ ] Usage statistics are captured and reported
- [ ] Error handling for model not found, invalid parameters
- [ ] Tests cover various parameter combinations

## Dependencies
- Requires: OpenAI HTTP Client implementation

## Estimate
**Priority:** High  
**Effort:** 3-4 days" \
    --label "enhancement,core,high-priority" \
    --milestone "v1.0.0"

gh issue create \
    --title "📡 Implement Streaming Support" \
    --body "## Description
Add streaming support for real-time responses from OpenAI API.

## Tasks
- [ ] Implement Server-Sent Events (SSE) parsing
- [ ] Add streaming request handling
- [ ] Create streaming response callbacks
- [ ] Handle partial JSON chunks
- [ ] Add stream completion detection
- [ ] Error handling during streaming
- [ ] Cancellation support

## Acceptance Criteria
- [ ] Can receive streaming responses from OpenAI
- [ ] Callbacks are triggered for each chunk
- [ ] Proper completion detection
- [ ] Graceful error handling during streams
- [ ] Stream cancellation works correctly
- [ ] Tests cover streaming scenarios

## Dependencies
- Requires: OpenAI HTTP Client implementation
- Requires: Chat Completions API

## Estimate
**Priority:** High  
**Effort:** 2-3 days" \
    --label "enhancement,streaming,high-priority" \
    --milestone "v1.0.0"

gh issue create \
    --title "🔒 Implement Authentication and API Key Management" \
    --body "## Description
Add robust authentication and API key management with security best practices.

## Tasks
- [ ] Environment variable support (OPENAI_API_KEY)
- [ ] Configuration file support
- [ ] API key validation
- [ ] Secure storage recommendations
- [ ] Multi-provider API key management
- [ ] Key rotation support
- [ ] Rate limiting awareness

## Acceptance Criteria
- [ ] API keys can be set via environment variables
- [ ] Keys are masked in logs and debug output
- [ ] Invalid keys are detected early
- [ ] Documentation includes security best practices
- [ ] Support for multiple provider keys
- [ ] Tests cover various auth scenarios

## Dependencies
- Related to: OpenAI HTTP Client implementation

## Estimate
**Priority:** Medium  
**Effort:** 1-2 days" \
    --label "enhancement,security,medium-priority" \
    --milestone "v1.0.0"

# Examples and Documentation
gh issue create \
    --title "📚 Create Comprehensive Examples" \
    --body "## Description
Create real-world examples showcasing library capabilities.

## Tasks
- [ ] Basic chat completion example
- [ ] Async request example
- [ ] Streaming response example
- [ ] Function calling example
- [ ] Error handling example
- [ ] Configuration management example
- [ ] Multi-provider example (when available)

## Acceptance Criteria
- [ ] All examples compile and run successfully
- [ ] Examples demonstrate best practices
- [ ] Clear comments explaining each step
- [ ] README includes example descriptions
- [ ] Examples work with CI/CD pipeline

## Dependencies
- Requires: Core OpenAI implementation
- Requires: Streaming support

## Estimate
**Priority:** Medium  
**Effort:** 1-2 days" \
    --label "documentation,examples,medium-priority" \
    --milestone "v1.0.0"

gh issue create \
    --title "📖 Add API Documentation with Doxygen" \
    --body "## Description
Generate comprehensive API documentation using Doxygen.

## Tasks
- [ ] Create Doxygen configuration file
- [ ] Add detailed class and method documentation
- [ ] Document all public APIs
- [ ] Add usage examples in documentation
- [ ] Configure GitHub Pages deployment
- [ ] Add documentation CI/CD integration

## Acceptance Criteria
- [ ] All public APIs are documented
- [ ] Documentation includes usage examples
- [ ] Doxygen generates clean HTML output
- [ ] Documentation is automatically deployed
- [ ] Documentation links work correctly

## Dependencies
- Requires: Core implementation complete

## Estimate
**Priority:** Medium  
**Effort:** 2-3 days" \
    --label "documentation,medium-priority" \
    --milestone "v1.0.0"

# Testing and Quality
gh issue create \
    --title "🧪 Add Integration Tests" \
    --body "## Description
Create integration tests that verify end-to-end functionality with real APIs.

## Tasks
- [ ] Set up test API keys (with limited permissions)
- [ ] Create integration test suite
- [ ] Test real OpenAI API interactions
- [ ] Add performance benchmarks
- [ ] Test error scenarios (rate limits, network issues)
- [ ] Add CI integration for optional API tests

## Acceptance Criteria
- [ ] Integration tests verify real API functionality
- [ ] Tests can run with or without API keys
- [ ] Performance benchmarks establish baselines
- [ ] CI can run integration tests securely
- [ ] Tests cover error and edge cases

## Dependencies
- Requires: Core OpenAI implementation
- Requires: Authentication system

## Estimate
**Priority:** Medium  
**Effort:** 2-3 days" \
    --label "testing,integration,medium-priority" \
    --milestone "v1.1.0"

# Future Enhancements
gh issue create \
    --title "🤖 Add Anthropic Claude Support" \
    --body "## Description
Extend the library to support Anthropic's Claude API as a second provider.

## Tasks
- [ ] Research Anthropic API structure
- [ ] Implement AnthropicClient class
- [ ] Add Claude-specific models and parameters
- [ ] Update ClientFactory for multi-provider support
- [ ] Add Claude-specific examples
- [ ] Update documentation for multi-provider usage

## Acceptance Criteria
- [ ] Can use Claude API through same interface
- [ ] Provider selection works seamlessly
- [ ] Claude-specific features are supported
- [ ] Examples demonstrate provider switching
- [ ] Tests cover both OpenAI and Anthropic

## Dependencies
- Requires: Core architecture complete
- Requires: OpenAI implementation stable

## Estimate
**Priority:** Low  
**Effort:** 3-5 days" \
    --label "enhancement,provider,low-priority" \
    --milestone "v2.0.0"

gh issue create \
    --title "📦 Package Manager Integration" \
    --body "## Description
Add support for popular C++ package managers.

## Tasks
- [ ] Create vcpkg port file
- [ ] Add Conan recipe
- [ ] Create CPM integration
- [ ] Add installation documentation
- [ ] Verify packages work correctly
- [ ] Set up automated package publishing

## Acceptance Criteria
- [ ] Library can be installed via vcpkg
- [ ] Conan package works correctly
- [ ] CPM integration is simple
- [ ] Documentation covers all installation methods
- [ ] CI verifies package installations

## Dependencies
- Requires: Stable v1.0.0 release

## Estimate
**Priority:** Low  
**Effort:** 3-4 days" \
    --label "packaging,distribution,low-priority" \
    --milestone "v1.2.0"

gh issue create \
    --title "⚡ Performance Optimization and Benchmarks" \
    --body "## Description
Optimize performance and establish benchmarks for the library.

## Tasks
- [ ] Profile request/response performance
- [ ] Optimize JSON serialization
- [ ] Add connection pooling
- [ ] Implement request caching
- [ ] Create benchmark suite
- [ ] Memory usage optimization
- [ ] Async performance tuning

## Acceptance Criteria
- [ ] Benchmark suite measures key metrics
- [ ] Performance is competitive with other clients
- [ ] Memory usage is reasonable
- [ ] Connection pooling improves throughput
- [ ] Benchmarks run in CI for regression detection

## Dependencies
- Requires: Core implementation complete

## Estimate
**Priority:** Low  
**Effort:** 4-5 days" \
    --label "performance,optimization,low-priority" \
    --milestone "v1.3.0"

echo ""
echo "✅ GitHub issues created successfully!"
echo ""
echo "View issues at: https://github.com/$REPO/issues"
echo ""
echo "Next steps:"
echo "1. Review and prioritize the created issues"
echo "2. Assign team members to specific issues"
echo "3. Start with the high-priority core implementation"
echo "4. Set up project boards for better tracking" 