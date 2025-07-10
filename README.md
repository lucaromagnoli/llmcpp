# JUCE LLM Client

A JUCE-based C++ library for interacting with Language Model APIs including OpenAI, Anthropic, and other providers.

## Features

- **Multi-provider support**: OpenAI (with more providers coming soon)
- **Async requests**: Non-blocking API calls using JUCE's threading
- **Streaming support**: Real-time streaming responses from supported providers
- **Type-safe**: Strong C++ typing with JUCE's var system
- **Header-only core**: Easy integration into existing JUCE projects
- **Modern C++20**: Uses modern C++ features and patterns

## Quick Start

### As a Git Submodule

```bash
# Add as submodule
git submodule add https://github.com/your-org/juce-llm-client.git third_party/juce-llm-client

# In your CMakeLists.txt
add_subdirectory(third_party/juce-llm-client)
target_link_libraries(your_target PRIVATE juce_llm_client)
```

### Basic Usage

```cpp
#include <juce_llm_client.h>

// Create OpenAI client
OpenAIClient client("your-api-key");

// Prepare request
LLMRequestConfig config;
config.client = "OpenAI";
config.model = "gpt-4o";
config.jsonSchema = R"({"type": "object", "properties": {"response": {"type": "string"}}})";

LLMRequest request{config, "Hello, world!", {}, ""};

// Send async request
client.sendRequest(request, [](LLMResponse response) {
    if (response.success) {
        DBG("Response: " + response.result.toString());
    } else {
        DBG("Error: " + response.errorMessage);
    }
});
```

### Streaming

```cpp
client.sendStreamingRequest(request, 
    // On completion
    [](LLMResponse response) {
        DBG("Stream completed");
    },
    // On chunk
    [](const juce::String& chunk) {
        DBG("Chunk: " + chunk);
    }
);
```

## Architecture

### Core Types

- `LLMRequest`: Request configuration and data
- `LLMResponse`: Response with result, success status, and metadata
- `LLMClient`: Abstract base class for all providers
- `LLMRequestConfig`: Configuration for model, parameters, etc.

### Providers

- **OpenAIClient**: Supports both Chat Completions and Responses APIs
- More providers coming soon (Anthropic, local models, etc.)

### Provider Management

- `ClientFactory`: Create clients with configuration
- `ClientManager`: Manage multiple clients and API keys

## Building

### Requirements

- CMake 3.22+
- C++20 compiler
- JUCE framework

### Build Options

```bash
cmake -DJUCE_LLM_CLIENT_BUILD_EXAMPLES=ON -DJUCE_LLM_CLIENT_BUILD_TESTS=ON ..
```

## Contributing

This library is designed to be open-sourced. Contributions welcome!

### Adding New Providers

1. Inherit from `LLMClient`
2. Implement required virtual methods
3. Add provider-specific types in your namespace
4. Update `ClientFactory` and `ClientManager`

## License

[License TBD - suggest MIT or similar permissive license for open source] 