# llmcpp

[![CI](https://github.com/lucaromagnoli/llmcpp/actions/workflows/ci.yml/badge.svg)](https://github.com/lucaromagnoli/llmcpp/actions/workflows/ci.yml)
[![Release](https://github.com/lucaromagnoli/llmcpp/actions/workflows/release.yml/badge.svg)](https://github.com/lucaromagnoli/llmcpp/actions/workflows/release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.22%2B-green.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](https://github.com/lucaromagnoli/llmcpp)

A modern C++20 library providing a unified interface for Large Language Model APIs, with support for async operations and multiple providers.

## Features

- **🚀 Modern C++20**: Uses latest C++ features and standard library
- **🔄 Multi-provider support**: OpenAI, Anthropic Claude
- **⚡ Async requests**: Non-blocking API calls using std::future
- **🔒 Type-safe**: Strong C++ typing with nlohmann/json
- **🎯 Header-only friendly**: Easy integration into any C++ project
- **🌐 Cross-platform**: Works on Linux, macOS, and Windows
- **✅ Production ready**: Full OpenAI Responses API implementation
- **📝 Flexible input**: Support for both simple prompts and structured context
- **🎯 Type-safe models**: Strongly typed Model enum for compile-time safety
- **📊 Performance benchmarks**: Comprehensive model comparison and cost analysis
- **🔌 MCP Integration**: Model Context Protocol support for external tool integration

## Quick Start

### Basic Usage

```cpp
#include <llmcpp.h>

int main() {
    // Create OpenAI client
    OpenAIClient client("your-openai-api-key");

    // Or create Anthropic client
    llmcpp::AnthropicClient anthropicClient("your-anthropic-api-key");

    // Method 1: Using Model enum (recommended - type-safe)
    auto response = client.sendRequest(OpenAI::Model::GPT_4o_Mini, "Hello! How are you?");

    // Method 2: Using traditional config approach
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o-mini";
    config.maxTokens = 100;
    config.temperature = 0.7f;

    LLMRequest request(config, "Hello! How are you?");
    auto response2 = client.sendRequest(request);

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
// Method 1: Using Model enum with context
LLMContext context = {
    {{"role", "user"}, {"content", "What is 2+2?"}}
};

std::string prompt = "You are a math assistant. Answer with just the number.";
auto response = client.sendRequest(OpenAI::Model::GPT_4o_Mini, prompt, context, 200, 0.1f);

// Method 2: Using traditional config approach
LLMRequestConfig config;
config.client = "openai";
config.model = "gpt-4o-mini";
config.maxTokens = 200;

LLMRequest request(config, prompt, context);
auto response2 = client.sendRequest(request);
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

---

### Model Enum: Type-Safe Model Selection

llmcpp provides a strongly-typed `OpenAI::Model` enum for selecting models safely and with IDE autocompletion. Example usage:

```cpp
// Use any supported model from the enum
auto response = client.sendRequest(OpenAI::Model::O3, "What's new in the O3 model?");
auto response2 = client.sendRequest(OpenAI::Model::GPT_4_1_Mini, "Summarize this text.");
```

**Available models:**
- GPT_5, GPT_5_Mini, GPT_5_Nano
- O3, O3_Mini
- O1, O1_Mini, O1_Preview, O1_Pro
- O4_Mini, O4_Mini_Deep_Research
- GPT_4_1, GPT_4_1_Mini, GPT_4_1_Nano
- GPT_4o, GPT_4o_Mini
- GPT_4_5 (preview)
- GPT_3_5_Turbo (legacy)
- Custom (for custom/unknown model names)

> **Note:** Model recommendations change frequently. For the latest guidance on which model to use for your use case (reasoning, coding, cost, etc.), consult the [OpenAI model documentation](https://platform.openai.com/docs/models) or your provider's docs. The library no longer provides a built-in recommendation function.

---

## Anthropic Claude

**llmcpp** now includes full support for [Anthropic's Claude models](https://docs.anthropic.com/en/docs/about-claude/models/overview) via the Messages API.

### Basic Anthropic Usage

```cpp
#include <llmcpp.h>

int main() {
    // Create Anthropic client
    llmcpp::AnthropicClient client("your-anthropic-api-key");

    // Method 1: Using Model enum (recommended)
    LLMRequestConfig config;
    config.model = Anthropic::toString(Anthropic::Model::CLAUDE_HAIKU_3_5);
    config.maxTokens = 100;

    LLMRequest request(config, "Write a haiku about programming.");
    auto response = client.sendRequest(request);

    if (response.success) {
        std::cout << "Response: " << response.result["text"].get<std::string>() << std::endl;
        std::cout << "Usage: " << response.usage.toString() << std::endl;
    }

    return 0;
}
```

### Direct Anthropic Messages API

```cpp
// Use the native Anthropic Messages API
Anthropic::MessagesRequest request;
request.model = "claude-3-5-sonnet-20241022";
request.maxTokens = 150;

// Add user message
Anthropic::Message userMsg;
userMsg.role = Anthropic::MessageRole::USER;
userMsg.content.push_back({.type = "text", .text = "Explain quantum computing"});
request.messages.push_back(userMsg);

auto response = client.sendMessagesRequest(request);

for (const auto& content : response.content) {
    if (content.type == "text") {
        std::cout << content.text << std::endl;
    }
}
```

### Available Claude Models

Based on the [official Anthropic documentation](https://docs.anthropic.com/en/docs/about-claude/models/overview):

**Claude 4 series (Latest - 2025):**
- CLAUDE_OPUS_4_1 (claude-opus-4-1-20250805) - Most capable and intelligent
- CLAUDE_OPUS_4 (claude-opus-4-20250514) - Previous flagship model
- CLAUDE_SONNET_4 (claude-sonnet-4-20250514) - High-performance model

**Claude 3.7 series:**
- CLAUDE_SONNET_3_7 (claude-3-7-sonnet-20250219) - High-performance with extended thinking

**Claude 3.5 series:**
- CLAUDE_SONNET_3_5_V2 (claude-3-5-sonnet-20241022) - Latest 3.5 Sonnet (upgraded)
- CLAUDE_SONNET_3_5 (claude-3-5-sonnet-20240620) - Previous 3.5 Sonnet
- CLAUDE_HAIKU_3_5 (claude-3-5-haiku-20241022) - Fastest model

**Claude 3 series (Legacy):**
- CLAUDE_OPUS_3 (claude-3-opus-20240229) - Legacy opus
- CLAUDE_HAIKU_3 (claude-3-haiku-20240307) - Fast and compact legacy model

### Using ClientFactory with Anthropic

```cpp
// Create client via factory
auto client = llmcpp::ClientFactory::createClient("anthropic", "your-api-key");

// Use common LLMRequest interface
LLMRequestConfig config;
config.model = "claude-3-5-haiku-20241022";
config.maxTokens = 100;

LLMRequest request(config, "Hello, Claude!");
auto response = client->sendRequest(request);
```

> **Note:** For the latest Claude model recommendations and capabilities, consult the [Anthropic documentation](https://docs.anthropic.com/en/docs/about-claude/models/overview).

---

## 🚀 Performance Benchmarks

The `llmcpp` library includes comprehensive benchmarks comparing OpenAI and Anthropic models across different tasks. Run benchmarks with:

```bash
# Set environment variables
export OPENAI_API_KEY="your-openai-key"
export ANTHROPIC_API_KEY="your-anthropic-key"
export LLMCPP_RUN_BENCHMARKS=1

# Run unified benchmarks
./tests/llmcpp_tests "[unified][benchmark]"
```

### 🏆 Performance Leaders

Based on real API testing with consistent Responses API usage:

| Category | Winner | Latency | Throughput | Cost-Effectiveness |
|----------|--------|---------|------------|-------------------|
| **Simple Text** | `gpt-4o-mini` | 1.2s | 26.2 tok/s | $0.0021/10K tokens |
| **Structured Output** | `gpt-4o-mini` | 1.2s | 51.7 tok/s | Excellent |
| **Reasoning Tasks** | `gpt-5` | 1.9s | 13.3 tok/s | Premium |
| **Premium Quality** | `claude-opus-4-1` | 2.7s | 25.2 tok/s | $0.225/10K tokens |

### 📊 Detailed Benchmark Results

#### Simple Text Generation
```
Provider    Model                  Latency   Tokens/sec   Cost-Effectiveness
────────────────────────────────────────────────────────────────────────────
OpenAI      gpt-4o-mini           1.22s     26.2        ⭐⭐⭐⭐⭐
Anthropic   claude-3-5-sonnet-v2  1.51s     21.8        ⭐⭐⭐⭐
OpenAI      gpt-5                 1.89s     13.3        ⭐⭐⭐
Anthropic   claude-sonnet-4       1.94s     34.5        ⭐⭐⭐
OpenAI      gpt-4o                2.25s     16.9        ⭐⭐⭐
Anthropic   claude-opus-4-1       2.66s     25.2        ⭐⭐
```

#### Structured JSON Output
```
Provider    Model                  Latency   Tokens/sec   Schema Validation
─────────────────────────────────────────────────────────────────────────────
OpenAI      gpt-4o-mini           1.24s     51.7        ✅ Strict Schema
Anthropic   claude-3-5-sonnet-v2  1.35s     28.9        ✅ Natural JSON
OpenAI      gpt-4o                1.50s     42.6        ✅ Strict Schema
Anthropic   claude-opus-4-1       1.76s     25.0        ✅ Natural JSON
OpenAI      gpt-5-mini            1.89s     38.1        ✅ Strict Schema
```

### 💡 Model Selection Guide

**For Cost-Conscious Applications:**
- **Winner:** `gpt-4o-mini` - Excellent performance at $0.0021 per 10K tokens
- **Alternative:** `claude-3-5-haiku` - Fast and affordable at $0.00375 per 10K tokens

**For Balanced Performance:**
- **Winner:** `claude-3-5-sonnet-v2` - Great quality/speed ratio
- **Alternative:** `gpt-4o` - Reliable with good structured output

**For Maximum Capability:**
- **Winner:** `claude-opus-4-1` - Highest reasoning and creative ability
- **Alternative:** `claude-sonnet-4` - Strong performance with good speed

**For Reasoning Tasks:**
- **Winner:** `gpt-5` with reasoning mode - Advanced logical thinking
- **Alternative:** `o3-mini` - Cost-effective reasoning capabilities

### 🔬 Benchmark Methodology

- **Consistent API Usage:** All tests use OpenAI Responses API for standardization
- **Real-World Conditions:** Actual API calls with network latency
- **Multiple Runs:** Results averaged across multiple test executions
- **Task Variety:** Simple text, structured output, and reasoning scenarios
- **Cost Analysis:** Based on current provider pricing (as of 2025)

### ⚡ Quick Performance Tips

1. **For Speed:** Use `gpt-4o-mini` for fastest responses
2. **For Cost:** Choose `claude-3-5-haiku` for budget-friendly options
3. **For Quality:** Select `claude-opus-4-1` when quality matters most
4. **For JSON:** Use OpenAI models with strict schema validation
5. **For Reasoning:** Enable `reasoning: {"effort": "low"}` for reasoning models

> **Note:** Benchmark results may vary based on network conditions, API load, and specific use cases. Run your own benchmarks for mission-critical applications.

---

## CMake Integration

### After install (recommended)

```cmake
find_package(llmcpp REQUIRED PATHS /path/to/install/lib/cmake/llmcpp)
target_link_libraries(my_target PRIVATE llmcpp::llmcpp)
```

- This will automatically add the correct include paths and link the library.
- Make sure nlohmann_json is installed on your system (e.g., via Homebrew or vcpkg).

### FetchContent/CPM (header-only or development)

```cmake
include(FetchContent)
FetchContent_Declare(
  llmcpp
  GIT_REPOSITORY https://github.com/lucaromagnoli/llmcpp.git
  GIT_TAG main
)
FetchContent_MakeAvailable(llmcpp)
target_link_libraries(my_target PRIVATE llmcpp)
```

- This is convenient for development, but for production use, prefer the install method above.

## Architecture

### Core Components

- **`OpenAIClient`**: Full OpenAI Responses API implementation
- **`LLMRequest`/`LLMResponse`**: Unified request/response types with flexible input mapping
- **`ClientManager`**: Smart pointer-based client management
- **`LLMRequestConfig`**: Configuration for models and parameters
- **`OpenAIMcpUtils`**: Model Context Protocol utilities for external tool integration

### Request Structure

The `LLMRequest` structure provides a unified interface that maps to provider-specific APIs:

- **`prompt`**: Main instructions/task (maps to OpenAI `instructions`)
- **`context`**: Vector of generic JSON objects (maps to OpenAI `input`)
- **`config`**: Model and parameter configuration

This design allows for:
- Simple text completion with just a prompt
- Structured conversations with context
- Provider-specific optimizations while maintaining a unified interface
- Type-safe model selection with IDE autocompletion
- Compile-time validation of model names

### Supported APIs

- **OpenAI Responses API**: ✅ Complete implementation
  - Sync and async requests
  - Structured outputs with JSON schema validation
  - Function calling and tool usage
  - Error handling and usage tracking
  - Flexible input mapping (prompt → instructions, context → input)
  - Model Context Protocol (MCP) integration

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

#### Available Models

The library provides type-safe model selection using the `OpenAI::Model` enum:

```cpp
// Available model enums
OpenAI::Model::GPT_5          // gpt-5 - Next-generation model (reasoning)
OpenAI::Model::GPT_5_Mini     // gpt-5-mini - Cost-effective GPT-5
OpenAI::Model::GPT_5_Nano     // gpt-5-nano - Fastest GPT-5
OpenAI::Model::GPT_4_1        // gpt-4.1 - Latest model with superior coding and structured outputs
OpenAI::Model::GPT_4_1_Mini   // gpt-4.1-mini - Balanced performance and cost
OpenAI::Model::GPT_4_1_Nano   // gpt-4.1-nano - Fastest and cheapest option
OpenAI::Model::GPT_4o         // gpt-4o - Good balance of performance and cost
OpenAI::Model::GPT_4o_Mini    // gpt-4o-mini - Cost-effective for basic tasks
OpenAI::Model::GPT_4_5        // gpt-4.5 - Preview model (deprecated July 2025)
OpenAI::Model::GPT_3_5_Turbo  // gpt-3.5-turbo - Legacy model
OpenAI::Model::Custom         // For custom model names
```

#### Model Selection Helpers

```cpp
// Get recommended model for specific use cases
auto codingModel = OpenAIClient::getRecommendedModelEnum("coding");           // GPT_4_1
auto costEffectiveModel = OpenAIClient::getRecommendedModelEnum("cost_effective"); // GPT_4_1_Mini
auto fastestModel = OpenAIClient::getRecommendedModelEnum("fastest");         // GPT_4_1_Nano

// Convert between enum and string
std::string modelStr = OpenAIClient::modelToString(OpenAI::Model::GPT_4o_Mini); // "gpt-4o-mini"
OpenAI::Model model = OpenAIClient::stringToModel("gpt-4.1");                   // GPT_4_1

// Check if model supports features
bool supportsStructured = OpenAI::supportsStructuredOutputs(OpenAI::Model::GPT_4_1); // true
```

### Structured Outputs

The library provides two ways to define JSON schemas for structured outputs:

#### Method 1: Manual JSON Schema (Basic)

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

#### Method 2: JsonSchemaBuilder (Recommended)

The `JsonSchemaBuilder` provides a fluent, type-safe API for building JSON schemas:

```cpp
#include <llmcpp/core/JsonSchemaBuilder.h>

// Build schema using fluent API
auto schema = JsonSchemaBuilder()
    .type("object")
    .property("answer", JsonSchemaBuilder()
        .type("string")
        .description("The analysis result")
        .required())
    .property("confidence", JsonSchemaBuilder()
        .type("number")
        .minimum(0.0)
        .maximum(1.0)
        .description("Confidence score between 0 and 1")
        .required())
    .property("tags", JsonSchemaBuilder()
        .type("array")
        .items(JsonSchemaBuilder().type("string"))
        .description("Optional tags for categorization"))
    .build();

LLMRequestConfig config;
config.schemaObject = schema;
config.functionName = "analyze_sentiment";

LLMRequest request(config, "Analyze the sentiment of this text");
auto response = client.sendRequest(request);
```

#### JsonSchemaBuilder Examples

```cpp
// Simple object with required fields
auto userSchema = JsonSchemaBuilder()
    .type("object")
    .property("name", JsonSchemaBuilder().type("string").required())
    .property("age", JsonSchemaBuilder().type("integer").minimum(0).required())
    .property("email", JsonSchemaBuilder().type("string").format("email"))
    .required({"name", "age"})
    .build();

// Array of objects
auto productsSchema = JsonSchemaBuilder()
    .type("array")
    .items(JsonSchemaBuilder()
        .type("object")
        .property("id", JsonSchemaBuilder().type("string").required())
        .property("name", JsonSchemaBuilder().type("string").required())
        .property("price", JsonSchemaBuilder().type("number").minimum(0).required())
        .required({"id", "name", "price"}))
    .build();

// Enum with specific values
auto statusSchema = JsonSchemaBuilder()
    .type("string")
    .enumValues({"pending", "approved", "rejected"})
    .description("Status of the request")
    .build();

// Conditional schema
auto conditionalSchema = JsonSchemaBuilder()
    .type("object")
    .property("type", JsonSchemaBuilder().type("string").required())
    .ifThen(
        JsonSchemaBuilder().property("type", JsonSchemaBuilder().constValue("user")),
        JsonSchemaBuilder().property("user_id", JsonSchemaBuilder().type("string").required())
    )
    .ifThen(
        JsonSchemaBuilder().property("type", JsonSchemaBuilder().constValue("admin")),
        JsonSchemaBuilder().property("admin_level", JsonSchemaBuilder().type("integer").minimum(1).required())
    )
    .required({"type"})
    .build();
```

#### Utility Methods

```cpp
// Common patterns
auto simpleString = JsonSchemaBuilder::string();
auto requiredString = JsonSchemaBuilder::requiredString();
auto optionalString = JsonSchemaBuilder::optionalString();
auto stringArray = JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string());
auto statusEnum = JsonSchemaBuilder::stringEnum({"active", "inactive", "pending"});
```

### Model Context Protocol (MCP) Integration

llmcpp includes utilities for integrating external tools via the Model Context Protocol:

```cpp
#include <openai/OpenAIMcpUtils.h>

// Convert MCP tools to OpenAI tool definitions
std::vector<json> mcpTools = getMcpToolsFromServer();
std::vector<OpenAI::ToolDefinition> openaiTools = OpenAI::convertMcpToolsToOpenAI(mcpTools);

// Add MCP tools to your request
LLMRequestConfig config;
config.model = "gpt-4o-mini";
config.tools = openaiTools;

LLMRequest request(config, "Your prompt here");
auto response = client.sendRequest(request);

// Handle tool calls
if (response.hasToolCalls()) {
    for (const auto& toolCall : response.getToolCalls()) {
        // Execute MCP tool and get result
        json toolResult = executeMcpTool(toolCall.name, toolCall.arguments);

        // Send result back to continue conversation
        // ...
    }
}
```

**MCP Features:**
- **Tool Discovery**: Automatically convert MCP tool definitions to OpenAI format
- **Type Mapping**: Seamless conversion between MCP and OpenAI schemas
- **Tool Execution**: Easy integration with MCP servers
- **Error Handling**: Robust error handling for MCP operations

For more details on MCP integration, see the [MCP integration tests](tests/integration/test_mcp_integration.cpp).

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
