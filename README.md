# llmcpp

A modern C++20 library providing a unified interface for Large Language Model APIs, with support for async operations and multiple providers.

## Features

- **🚀 Modern C++20**: Uses latest C++ features and standard library
- **🔄 Multi-provider support**: OpenAI (with more providers coming)
- **⚡ Async requests**: Non-blocking API calls using std::future
- **🔒 Type-safe**: Strong C++ typing with nlohmann/json
- **🎯 Header-only friendly**: Easy integration into any C++ project
- **🌐 Cross-platform**: Works on Linux, macOS, and Windows
- **✅ Production ready**: Full OpenAI Responses API implementation
- **📝 Flexible input**: Support for both simple prompts and structured context

## Quick Start

### Basic Usage

```cpp
#include <llmcpp.h>

int main() {
    // Create OpenAI client
    OpenAIClient client("your-api-key-here");

    // Configure request
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o-mini";
    config.maxTokens = 100;
    config.temperature = 0.7f;

    // Create and send request with prompt
    LLMRequest request(config, "Hello! How are you?");
    auto response = client.sendRequest(request);

    // Handle response
    if (response.success) {
        std::cout << "Response: " << response.result["text"].get<std::string>() << std::endl;
        std::cout << "Usage: " << response.usage.toString() << std::endl;
    } else {
        std::cerr << "Error: " << response.errorMessage << std::endl;
    }

    return 0;
}
```

### Using Context and Instructions

```cpp
// Create request with separate prompt (instructions) and context
LLMRequestConfig config;
config.client = "openai";
config.model = "gpt-4o-mini";
config.maxTokens = 200;

// Context data (will be mapped to OpenAI input)
LLMContext context = {
    {{"role", "user"}, {"content", "What is 2+2?"}}
};

// Prompt becomes instructions for the model
std::string prompt = "You are a math assistant. Answer with just the number.";

LLMRequest request(config, prompt, context);
auto response = client.sendRequest(request);
```

### Async Usage

```cpp
// Send async request with callback
auto future = client.sendRequestAsync(request, [](const LLMResponse& response) {
    if (response.success) {
        std::cout << "Async response: " << response.result["text"].get<std::string>() << std::endl;
    }
});

// Do other work while waiting
// ...

// Get final result
auto response = future.get();
```

### Using ClientManager

```cpp
#include <llmcpp.h>

int main() {
    ClientManager manager;

    // Create and register OpenAI client
    auto client = manager.createClient<OpenAIClient>("openai", "your-api-key");

    // Use the client
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o-mini";
    config.maxTokens = 50;

    LLMRequest request(config, "Hello world!");
    auto response = client->sendRequest(request);

    return 0;
}
```

## Architecture

### Core Components

- **`OpenAIClient`**: Full OpenAI Responses API implementation
- **`LLMRequest`/`LLMResponse`**: Unified request/response types with flexible input mapping
- **`ClientManager`**: Smart pointer-based client management
- **`LLMRequestConfig`**: Configuration for models and parameters

### Request Structure

The `LLMRequest` structure provides a unified interface that maps to provider-specific APIs:

- **`prompt`**: Main instructions/task (maps to OpenAI `instructions`)
- **`context`**: Vector of generic JSON objects (maps to OpenAI `input`)
- **`config`**: Model and parameter configuration

This design allows for:
- Simple text completion with just a prompt
- Structured conversations with context
- Provider-specific optimizations while maintaining a unified interface

### Supported APIs

- **OpenAI Responses API**: ✅ Complete implementation
  - Sync and async requests
  - Structured outputs with JSON schema validation
  - Function calling and tool usage
  - Error handling and usage tracking
  - Flexible input mapping (prompt → instructions, context → input)

## Building

### Requirements

- CMake 3.22+
- C++20 compiler (GCC 10+, Clang 12+, MSVC 2019+)
- OpenSSL (for HTTPS support)
- 🥷 Ninja build system (recommended, auto-installed via CMake)

### Build with Make

```bash
# Quick build and test
make

# Build with tests
make tests

# Run tests
make test-unit           # Unit tests only
make test-integration    # Integration tests (requires API key)
make test-ci            # CI-safe tests (no API calls)

# Show all targets
make help
```

### Build with CMake

```bash
mkdir build && cd build
cmake .. -DLLMCPP_BUILD_TESTS=ON
cmake --build .
```

**Note**: The project uses Ninja as the default build system for faster builds. CMake will automatically download and use Ninja if available. To use a different generator, specify it explicitly:

```bash
# Use Makefiles instead
cmake .. -G "Unix Makefiles" -DLLMCPP_BUILD_TESTS=ON
cmake --build .

# Use Visual Studio on Windows
cmake .. -G "Visual Studio 17 2022" -DLLMCPP_BUILD_TESTS=ON
cmake --build .
```

## Integration

### As Git Submodule

```bash
git submodule add https://github.com/lucaromagnoli/llmcpp.git third_party/llmcpp
```

```cmake
add_subdirectory(third_party/llmcpp)
target_link_libraries(your_target PRIVATE llmcpp)
```

### With CMake FetchContent

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

## Configuration

### API Keys

```cpp
// Environment variable (recommended)
const char* apiKey = std::getenv("OPENAI_API_KEY");
OpenAIClient client(apiKey);

// Or set directly
OpenAIClient client("your-api-key-here");
```

### Model Configuration

```cpp
LLMRequestConfig config;
config.model = "gpt-4o-mini";     // Model name - cost-effective for basic tasks
config.maxTokens = 500;           // Max response tokens
config.temperature = 0.7f;         // Temperature (0.0 - 1.0)
config.functionName = "my_func";  // Function name for structured outputs
```

#### Recommended Models

- **`gpt-4.1`**: Latest model with superior coding, instruction following, and structured outputs (1M context)
- **`gpt-4.1-mini`**: Balanced performance and cost, beats gpt-4o in many benchmarks
- **`gpt-4.1-nano`**: Fastest and cheapest option for simple tasks like classification
- **`gpt-4o-mini`**: Cost-effective for basic text completion tasks
- **`gpt-4o`**: Good balance of performance and cost for general use

### Structured Outputs

```cpp
// Define JSON schema for structured output
json schema = {
    {"type", "object"},
    {"properties", {
        {"answer", {{"type", "string"}}},
        {"confidence", {{"type", "number"}}}
    }},
    {"required", {"answer", "confidence"}}
};

LLMRequestConfig config;
config.schemaObject = schema;
config.functionName = "analyze_sentiment";

LLMRequest request(config, "Analyze the sentiment of this text");
auto response = client.sendRequest(request);

// Access structured output
if (response.success) {
    auto result = response.result["text"];
    std::cout << "Answer: " << result["answer"] << std::endl;
    std::cout << "Confidence: " << result["confidence"] << std::endl;
}
```

## Testing

### Unit Tests

```bash
make test-unit
```

### Integration Tests

Set up your API key and run integration tests:

```bash
# Using environment variables
export OPENAI_API_KEY="your-api-key"
export LLMCPP_RUN_INTEGRATION_TESTS=1
make test-integration

# Or using .env file
echo "OPENAI_API_KEY=your-api-key" > .env
echo "LLMCPP_RUN_INTEGRATION_TESTS=1" >> .env
make test-integration
```

**Note**: Integration tests make real API calls and will incur charges. They are disabled by default and excluded from CI.

## Examples

See the `examples/` directory for complete working examples:

- `basic_usage.cpp`: Basic synchronous usage
- `async_example.cpp`: Async requests with callbacks

Build and run examples:

```bash
mkdir build && cd build
cmake .. -DLLMCPP_BUILD_EXAMPLES=ON
cmake --build .
./examples/basic_usage
```

## Dependencies

Dependencies are automatically managed via CMake FetchContent:

- **nlohmann/json**: JSON parsing and manipulation
- **cpp-httplib**: HTTP/HTTPS client
- **Catch2**: Testing framework (tests only)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new features
5. Ensure all tests pass (`make test-ci`)
6. Submit a pull request

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.
