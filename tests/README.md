# llmcpp Tests

This directory contains the test suite for the llmcpp library, organized into unit tests and integration tests.

## Test Structure

```
tests/
├── unit/                      # Unit tests (fast, no external dependencies)
│   ├── test_types.cpp         # Core type tests
│   ├── test_client.cpp        # Client interface tests
│   ├── test_llm_types.cpp     # LLM types and utilities
│   ├── test_openai_response_parsing.cpp  # JSON parsing logic
│   ├── test_client_factory.cpp           # Client factory tests
│   ├── test_openai_types.cpp             # OpenAI type definitions (disabled)
│   └── test_openai_client.cpp            # OpenAI client unit tests (disabled)
└── integration/               # Integration tests (slower, external APIs)
    ├── test_simple_integration.cpp       # Basic integration tests
    └── test_openai_integration.cpp       # OpenAI API integration (disabled)
```

## Running Tests

### Makefile Targets

The project provides several make targets for running different test categories:

```bash
# Run all tests (including integration if configured)
make test

# Run unit tests only (excludes integration tests)
make test-unit

# Run parsing tests only
make test-parsing

# Run OpenAI-related tests (excluding integration)
make test-openai

# Run integration tests only
make test-integration

# Run CI-safe tests (excludes integration tests)
make test-ci
```

### Direct Test Execution

You can also run tests directly using the test executable:

```bash
# Build tests first
make tests

# Run all tests
./build/tests/llmcpp_tests

# Run specific test categories using filters
./build/tests/llmcpp_tests "~[integration]"     # Exclude integration tests
./build/tests/llmcpp_tests "[parsing]"          # Only parsing tests
./build/tests/llmcpp_tests "[openai]"           # Only OpenAI tests
./build/tests/llmcpp_tests "[integration]"      # Only integration tests
```

## Test Categories

Tests are organized using Catch2 tags:

- `[llm][types]` - Core LLM type tests
- `[client]` - Client interface and factory tests
- `[openai][parsing]` - OpenAI response parsing tests
- `[integration]` - Tests that require external API calls
- `[info]` - Informational tests (always pass)

## Integration Tests

Integration tests make real API calls and are disabled by default to:
- Avoid unexpected costs
- Prevent failures due to network issues
- Keep CI builds fast and reliable

### Running Integration Tests

To run integration tests, you need:

1. **API Key**: Set your OpenAI API key
2. **Enable Flag**: Enable integration tests

#### Option 1: Environment Variables
```bash
export OPENAI_API_KEY='sk-your-api-key-here'
export LLMCPP_RUN_INTEGRATION_TESTS=1
make test-integration
```

#### Option 2: .env File
```bash
echo 'OPENAI_API_KEY=sk-your-api-key-here' > .env
echo 'LLMCPP_RUN_INTEGRATION_TESTS=1' >> .env
make test-integration
```

> ⚠️ **Warning**: Integration tests make real API calls and will incur charges!
> 💡 **Tip**: Use `gpt-4o-mini` for cheaper testing

## CI Configuration

The CI system runs only unit tests to ensure:
- Fast build times
- No external dependencies
- No API costs
- Reliable builds

The CI uses the `test-ci` target which excludes all `[integration]` tagged tests.

## Adding New Tests

### Unit Tests
1. Add your test file to `tests/unit/`
2. Update `tests/CMakeLists.txt` to include the new file in `UNIT_TEST_SOURCES`
3. Use appropriate tags: `[llm]`, `[client]`, `[openai]`, etc.

### Integration Tests
1. Add your test file to `tests/integration/`
2. Update `tests/CMakeLists.txt` to include the new file in `INTEGRATION_TEST_SOURCES`
3. Always tag with `[integration]`
4. Include proper API key and flag checks at the beginning

### Example Test Structure

```cpp
#include <catch2/catch_test_macros.hpp>
#include "your_header.h"

TEST_CASE("Your test description", "[category][subcategory]") {
    SECTION("Test section") {
        // Your test code
        REQUIRE(someCondition == true);
    }
}
```

## Test Dependencies

- **Catch2**: Testing framework (v3.5.2)
- **nlohmann/json**: JSON processing
- **OpenSSL**: For HTTPS requests in integration tests

All dependencies are automatically fetched by CMake using FetchContent.