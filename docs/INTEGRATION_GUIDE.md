# llmcpp Integration Guide

## Quick Start Integration

This guide helps you integrate the llmcpp library into your C++ project for OpenAI API interactions with structured outputs.

## Installation

### Option 1: CMake FetchContent (Recommended)

```cmake
# In your CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
    llmcpp
    GIT_REPOSITORY https://github.com/lucaromagnoli/llmcpp.git
    GIT_TAG main  # or specific version tag
)

FetchContent_MakeAvailable(llmcpp)

# Link to your target
target_link_libraries(your_target_name PRIVATE llmcpp)
```

### Option 2: Git Submodule

```bash
git submodule add https://github.com/lucaromagnoli/llmcpp.git third_party/llmcpp
```

```cmake
# In your CMakeLists.txt
add_subdirectory(third_party/llmcpp)
target_link_libraries(your_target_name PRIVATE llmcpp)
```

## Basic Usage

### 1. Include Headers

```cpp
#include "core/ClientManager.h"
#include "openai/OpenAISchemaBuilder.h"
#include "core/JsonSchemaBuilder.h"
```

### 2. Initialize Client

```cpp
#include "core/ClientManager.h"

// Initialize the client manager
ClientManager manager;

// Set your OpenAI API key
std::string apiKey = "your-openai-api-key";
auto client = manager.createClient("openai", apiKey);
```

### 3. Simple Text Generation

```cpp
#include "core/LLMTypes.h"

// Create a simple request
LLMRequest request;
request.messages = {{"user", "Explain C++ smart pointers in one sentence."}};
request.config.model = "gpt-4o-mini";
request.config.temperature = 0.7;
request.config.maxTokens = 100;

// Send request
auto response = client->sendRequestSync(request);

if (response.success) {
    std::cout << "Response: " << response.result << std::endl;
} else {
    std::cerr << "Error: " << response.error << std::endl;
}
```

### 4. Structured Output (JSON Schema)

```cpp
#include "openai/OpenAISchemaBuilder.h"

// Create a schema for structured output
auto schema = OpenAIResponsesSchemaBuilder("analyze_content")
    .description("Analyze content and extract structured information")
    .property("category", JsonSchemaBuilder::stringEnum({"tech", "science", "business", "entertainment"}))
    .property("sentiment", JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
    .property("key_topics", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
    .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
    .required({"category", "sentiment"})
    .build();

// Create request with structured output
LLMRequest request;
request.messages = {{"user", "Analyze this content: [your content here]"}};
request.config.model = "gpt-4.1";  // Use GPT-4.1 for structured outputs
request.config.temperature = 0.1;
request.config.textOutputConfig = schema;

// Send request
auto response = client->sendRequestSync(request);

if (response.success) {
    // Parse the structured JSON response
    auto result = json::parse(response.result);
    std::cout << "Category: " << result["category"] << std::endl;
    std::cout << "Sentiment: " << result["sentiment"] << std::endl;
    std::cout << "Confidence: " << result["confidence"] << std::endl;
}
```

## Common Schema Examples

### Sentiment Analysis

```cpp
auto sentimentSchema = OpenAIResponsesSchemaBuilder("sentiment_analysis")
    .description("Analyze sentiment of text")
    .property("sentiment", JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
    .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
    .property("reasoning", JsonSchemaBuilder::string())
    .required({"sentiment", "confidence"})
    .build();
```

### Data Extraction

```cpp
auto extractionSchema = OpenAIResponsesSchemaBuilder("extract_data")
    .description("Extract structured data from text")
    .property("title", JsonSchemaBuilder::string())
    .property("author", JsonSchemaBuilder::string())
    .property("date", JsonSchemaBuilder::string())
    .property("tags", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
    .property("summary", JsonSchemaBuilder::string().maxLength(200))
    .required({"title", "summary"})
    .build();
```

### Classification

```cpp
auto classificationSchema = OpenAIResponsesSchemaBuilder("classify_content")
    .description("Classify content into categories")
    .property("primary_category", JsonSchemaBuilder::stringEnum({
        "technology", "science", "business", "entertainment", "sports", "politics"
    }))
    .property("secondary_categories", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
    .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
    .property("reasoning", JsonSchemaBuilder::string())
    .required({"primary_category", "confidence"})
    .build();
```

## Environment Setup

### 1. API Key Configuration

```cpp
// Option 1: Environment variable
std::string apiKey = std::getenv("OPENAI_API_KEY");

// Option 2: Configuration file
// Create a config.json file:
{
    "openai_api_key": "your-key-here",
    "default_model": "gpt-4o-mini",
    "max_tokens": 1000
}
```

### 2. Error Handling Pattern

```cpp
class AIService {
private:
    std::unique_ptr<LLMClient> client;

public:
    AIService(const std::string& apiKey) {
        ClientManager manager;
        client = manager.createClient("openai", apiKey);
    }

    std::optional<json> analyzeContent(const std::string& input,
                                     const OpenAI::TextOutputConfig& schema) {
        try {
            LLMRequest request;
            request.messages = {{"user", input}};
            request.config.model = "gpt-4.1";
            request.config.temperature = 0.1;
            request.config.textOutputConfig = schema;

            auto response = client->sendRequestSync(request);

            if (response.success) {
                return json::parse(response.result);
            } else {
                std::cerr << "AI Analysis failed: " << response.error << std::endl;
                return std::nullopt;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception in analyzeContent: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};
```

## Advanced Features

### Async Processing

```cpp
#include <future>

// For non-blocking requests
client->sendRequest(request, [](const LLMResponse& response) {
    if (response.success) {
        // Process response in callback
        auto result = json::parse(response.result);
        // Handle result...
    }
});
```

### Multiple Model Support

```cpp
// Use different models for different tasks
LLMRequestConfig fastConfig;
fastConfig.model = "gpt-4o-mini";  // Fast, cheap for simple tasks
fastConfig.temperature = 0.1;

LLMRequestConfig advancedConfig;
advancedConfig.model = "gpt-4.1";  // Advanced for complex analysis
advancedConfig.temperature = 0.1;
```

### Batch Processing

```cpp
std::vector<std::string> texts = {"text1", "text2", "text3"};
std::vector<std::future<LLMResponse>> futures;

for (const auto& text : texts) {
    LLMRequest request;
    request.messages = {{"user", "Analyze: " + text}};
    request.config = fastConfig;

    // Send async requests
    futures.push_back(std::async(std::launch::async, [&]() {
        return client->sendRequestSync(request);
    }));
}

// Collect results
for (auto& future : futures) {
    auto response = future.get();
    // Process each response...
}
```

## Dependencies

The library handles these dependencies automatically:
- **nlohmann/json** - JSON parsing
- **OpenSSL** - HTTPS requests
- **Threads** - Async support

## Models to Use

- **gpt-4o-mini**: Fast, cheap for simple tasks
- **gpt-4.1**: Advanced reasoning, structured outputs
- **gpt-4.1-mini**: Balanced performance/cost

## Best Practices

1. **Always validate structured outputs** - Parse JSON and check required fields
2. **Use appropriate models** - gpt-4o-mini for simple tasks, gpt-4.1 for complex analysis
3. **Handle errors gracefully** - Network issues, API limits, invalid responses
4. **Cache expensive operations** - Don't re-analyze the same content
5. **Set reasonable timeouts** - Prevent hanging requests
6. **Monitor API usage** - Track costs and rate limits

## Integration Checklist

- [ ] Add llmcpp to CMakeLists.txt
- [ ] Set up OpenAI API key
- [ ] Create basic client initialization
- [ ] Define JSON schemas for your use cases
- [ ] Implement error handling
- [ ] Add unit tests for AI service classes
- [ ] Set up async processing if needed
- [ ] Add logging for debugging
- [ ] Configure rate limiting if needed
- [ ] Test with real API calls
