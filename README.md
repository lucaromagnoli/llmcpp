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
auto client = llmcpp::ClientFactory::createOpenAIClient("your-api-key");

// Prepare request
llmcpp::Config config;
config.client = "openai";
config.model = "gpt-4";
config.maxTokens = 100;
config.jsonSchema = nlohmann::json::object({
    {"type", "object"},
    {"properties", {
        {"response", {{"type", "string"}}}
    }}
});

llmcpp::Request request;
request.config = config;
request.prompt = "Hello, world!";

// Send synchronous request
auto response = client->sendRequest(request);
if (response.success) {
    std::cout << "Response: " << response.result.dump() << std::endl;
} else {
    std::cerr << "Error: " << response.errorMessage << std::endl;
}
```

### Async Usage

```cpp
// Send async request
auto future = client->sendRequestAsync(request, [](llmcpp::Response response) {
    if (response.success) {
        std::cout << "Async response: " << response.result.dump() << std::endl;
    }
});

// Wait for completion
auto response = future.get();
```

### Streaming

```cpp
auto future = client->sendStreamingRequest(request,
    // Stream callback - called for each chunk
    [](const std::string& chunk) {
        std::cout << "Chunk: " << chunk << std::endl;
    },
    // Final callback - called when complete
    [](llmcpp::Response response) {
        std::cout << "Stream completed" << std::endl;
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

- **OpenAIClient**: Supports Chat Completions and function calling
- More providers coming soon (Anthropic, local models, etc.)

### Provider Management

- `ClientFactory`: Create clients with configuration
- `ClientManager`: Manage multiple clients and API keys

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

// Or programmatically
client->setApiKey("your-key-here");
```

### Model Configuration

```cpp
llmcpp::Config config;
config.model = "gpt-4";           // Model name
config.maxTokens = 500;           // Max response tokens
config.randomness = 0.7f;         // Temperature (0.0 - 1.0)
config.functionName = "my_func";  // Function calling name
```

## Examples

See the `examples/` directory for complete usage examples:

- **basic_usage.cpp**: Simple sync/async requests
- **streaming.cpp**: Streaming responses
- **function_calling.cpp**: Using structured outputs
- **error_handling.cpp**: Comprehensive error handling

## Contributing

This library is open source and contributions are welcome!

### Adding New Providers

1. Inherit from `LLMClient`
2. Implement all pure virtual methods
3. Add provider-specific types in your namespace
4. Update `ClientFactory` and `ClientManager`
5. Add tests and examples

### Development

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
make coverage                # Generate coverage report

# Alternative: Using CMake directly
mkdir build && cd build
cmake .. -DLLMCPP_BUILD_TESTS=ON -DLLMCPP_BUILD_EXAMPLES=ON
cmake --build .
ctest
```

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Roadmap

- [ ] Complete OpenAI implementation
- [ ] Add Anthropic Claude support
- [ ] Add local model support (llama.cpp integration)
- [ ] Add Azure OpenAI support
- [ ] Add batch processing
- [ ] Add response caching
- [ ] Add comprehensive logging
- [ ] Add metrics and monitoring hooks 