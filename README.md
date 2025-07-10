# llmcpp

A modern C++20 library providing a unified interface for Large Language Model APIs, with support for async operations, streaming, and multiple providers.

## Features

- **🚀 Modern C++20**: Uses latest C++ features and standard library
- **🔄 Multi-provider support**: OpenAI (with more providers coming soon)
- **⚡ Async requests**: Non-blocking API calls using std::future
- **📡 Streaming support**: Real-time streaming responses from supported providers
- **🔒 Type-safe**: Strong C++ typing with nlohmann/json
- **🎯 Header-only friendly**: Easy integration into any C++ project
- **🌐 No framework dependencies**: Works with any C++ codebase

## Quick Start

### As a Git Submodule

```bash
# Add as submodule
git submodule add https://github.com/lucaromagnoli/llmcpp.git third_party/llmcpp

# In your CMakeLists.txt
add_subdirectory(third_party/llmcpp)
target_link_libraries(your_target PRIVATE llmcpp)
```

### Basic Usage

```cpp
#include <llmcpp.h>

// Create OpenAI client
auto client = llmcpp::ClientFactory::createClient("openai", "your-api-key");

// Prepare request configuration
llmcpp::Config config;
config.client = "openai";
config.model = "gpt-4";
config.maxTokens = 100;
config.functionName = "my_function";

// Create request
llmcpp::Request request(config, "Hello, how are you?");

// Send synchronous request
client->sendRequest(request, [](const llmcpp::Response& response) {
    if (response.success) {
        std::cout << "Response: " << response.result.dump() << std::endl;
    } else {
        std::cerr << "Error: " << response.errorMessage << std::endl;
    }
});
```

### Async Usage

```cpp
// Send async request with callback
client->sendRequest(request, [](const llmcpp::Response& response) {
    if (response.success) {
        std::cout << "Async response: " << response.result.dump() << std::endl;
    }
});
```

### Streaming (Future Feature)

```cpp
// Streaming support (when implemented)
client->sendStreamingRequest(request,
    // Final callback - called when complete
    [](const llmcpp::Response& response) {
        std::cout << "Stream completed" << std::endl;
    },
    // Stream callback - called for each chunk
    [](const std::string& chunk) {
        std::cout << "Chunk: " << chunk << std::endl;
    }
);
```

## Architecture

### Core Types

- `LLMRequest`: Request configuration and data
- `LLMResponse`: Response with result, success status, and metadata  
- `LLMClient`: Abstract base class for all providers
- `LLMRequestConfig`: Configuration for model, parameters, etc.
- `LLMErrorCode`: Comprehensive error handling

### Providers

- **OpenAIClient**: Supports OpenAI Responses API and Chat Completions
- More providers coming soon (Anthropic, local models, etc.)

### Provider Management

- `ClientFactory`: Create clients with configuration
- `ClientManager`: Manage multiple clients and API keys

## Current Status

🚧 **This library is currently in active development (v1.0.0)**

**Working Features:**
- ✅ Project structure and build system
- ✅ Core type definitions and interfaces
- ✅ OpenAI client architecture
- ✅ Cross-platform CI/CD (Linux, macOS, Windows)

**In Progress:**
- 🔄 OpenAI HTTP client implementation
- 🔄 OpenAI Responses API integration
- 🔄 Async request handling
- 🔄 Streaming support

**Planned:**
- 📋 Chat Completions API
- 📋 Function calling support
- 📋 Additional providers (Anthropic, etc.)
- 📋 Examples and documentation

See [ROADMAP.md](ROADMAP.md) for detailed development plans.

## Building

### Requirements

- CMake 3.22+
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- OpenSSL (for HTTPS support)

### Dependencies

Dependencies are automatically fetched via CMake FetchContent:
- **nlohmann/json**: For JSON parsing and manipulation
- **cpp-httplib**: For HTTP/HTTPS requests

### Build Options

#### Using Make (Recommended)

```bash
# Quick start - build and test
make quick

# Build in release mode (default)
make

# Build with tests and examples
make all-features test

# Show all available targets
make help

# Development workflow
make dev                    # Build debug + examples + tests
make format                 # Format code
make lint                   # Run static analysis
make clean                  # Clean build
```

#### Using CMake Directly

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DLLMCPP_BUILD_EXAMPLES=ON -DLLMCPP_BUILD_TESTS=ON

# Build
cmake --build .
```

### Integration

#### CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    llmcpp
    GIT_REPOSITORY https://github.com/lucaromagnoli/llmcpp.git
    GIT_TAG main
)

FetchContent_MakeAvailable(llmcpp)
target_link_libraries(your_target PRIVATE llmcpp)
```

#### As Submodule

```bash
git submodule add https://github.com/lucaromagnoli/llmcpp.git third_party/llmcpp
```

```cmake
add_subdirectory(third_party/llmcpp)
target_link_libraries(your_target PRIVATE llmcpp)
```

## Configuration

### API Keys

Set your API keys securely:

```cpp
// Environment variable (recommended)
setenv("OPENAI_API_KEY", "your-key-here", 1);

// Or pass directly to factory
auto client = llmcpp::ClientFactory::createClient("openai", "your-api-key");
```

### Model Configuration

```cpp
llmcpp::Config config;
config.model = "gpt-4";           // Model name
config.maxTokens = 500;           // Max response tokens
config.randomness = 0.7f;         // Temperature (0.0 - 1.0)
config.functionName = "my_func";  // Function name for structured outputs
```

## Development

### Clone and Build

```bash
# Clone with submodules
git clone --recursive https://github.com/lucaromagnoli/llmcpp.git
cd llmcpp

# Quick development setup
make dev                     # Build debug + examples + tests

# Development workflow
make quick                   # Build and test
make format                  # Format code
make lint                    # Static analysis
make clean                   # Clean build
```

### Contributing

This library is open source and contributions are welcome!

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new features
5. Ensure all tests pass
6. Submit a pull request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Roadmap

### Phase 1: Core Implementation (v1.0.0)
- [ ] OpenAI HTTP Client
- [ ] OpenAI Responses API
- [ ] Streaming support
- [ ] Authentication and API key management
- [ ] Examples and documentation

### Phase 2: Enhanced Features (v1.1.0)
- [ ] Chat Completions API
- [ ] Function calling support
- [ ] Integration tests
- [ ] Performance optimizations

### Phase 3: Multi-Provider (v2.0.0)
- [ ] Anthropic Claude support
- [ ] Local model support (llama.cpp integration)
- [ ] Azure OpenAI support
- [ ] Batch processing

For detailed development plans, see [ROADMAP.md](ROADMAP.md). 