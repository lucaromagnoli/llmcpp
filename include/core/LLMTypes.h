#pragma once
#include <juce_core/juce_core.h>
#include <utility>

using LLMInput = juce::StringArray;

/// Represents the configuration for the LLM
struct LLMRequestConfig {
    juce::String client;
    juce::String model;
    juce::String functionName = "llm_function"; // Default function name for LLM calls
    juce::String jsonSchema{};

    float randomness = 0.8f;
    int maxTokens = 200;

    // Add more configuration options as needed (e.g., top_p, stop sequences, etc.)
};

struct LLMRequest {
    LLMRequest() = delete;
    LLMRequestConfig config;
    juce::String prompt;
    LLMInput inputValues;
    juce::String previousResponseId; // For native OpenAI conversation management

    [[nodiscard]] juce::String toString() const {
        juce::String inputValuesString = inputValues.joinIntoString(", ");
        return "LLMRequest {\n config: {\n client: " + config.client + ",\n model: " + config.model +
               ",\n functionName: " + config.functionName + ",\n jsonSchema: " + config.jsonSchema +
               ",\n randomness: " + juce::String(config.randomness) +
               ",\n maxTokens: " + juce::String(config.maxTokens) + "\n },\n prompt: " + prompt + ",\n inputValues: [" +
               inputValuesString + "],\n previousResponseId: " + previousResponseId + "\n}";
    }
};

struct LLMResponse {
    juce::var result{};
    bool success;
    juce::String errorMessage{};
    juce::String responseId; // For conversation continuity with OpenAI

    [[nodiscard]] juce::String toString() const {
        juce::String resultString;
        if (result.isObject() || result.isArray())
            resultString = juce::JSON::toString(result);
        else
            resultString = result.toString();
        return "LLMResponse {\n result: " + resultString + ",\n success: " + (success ? "true" : "false") +
               ",\n errorMessage: " + errorMessage + ",\n responseId: " + responseId + "\n}";
    }
};

struct LLMUsage {
    int inputTokens = 0;
    int outputTokens = 0;
    [[nodiscard]] int totalTokens() const { return inputTokens + outputTokens; }
};

// Error codes for LLM operations
enum class LLMErrorCode {
    None = 0,
    NetworkError,
    AuthenticationError,
    RateLimitError,
    InvalidRequest,
    ModelNotFound,
    InternalError,
    Unknown
};

// Optional callback types
using LLMResponseCallback = std::function<void(LLMResponse)>;
using LLMStreamCallback = std::function<void(const juce::String&)>; 