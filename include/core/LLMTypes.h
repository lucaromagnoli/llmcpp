#pragma once
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;

// Input type using standard C++ vectors instead of JUCE StringArray
using LLMInput = std::vector<std::string>;

/// Represents the configuration for the LLM
struct LLMRequestConfig {
    std::string client;
    std::string model;
    std::string functionName = "llm_function";  // Default function name for LLM calls
    std::string jsonSchema;

    float randomness = 0.8f;
    int maxTokens = 200;

    // Add more configuration options as needed (e.g., top_p, stop sequences, etc.)

    std::string toString() const {
        return "LLMRequestConfig { client: " + client + ", model: " + model +
               ", functionName: " + functionName + ", jsonSchema: " + jsonSchema +
               ", randomness: " + std::to_string(randomness) +
               ", maxTokens: " + std::to_string(maxTokens) + " }";
    }
};

struct LLMRequest {
    LLMRequest() = delete;
    LLMRequest(LLMRequestConfig config, std::string prompt, LLMInput inputValues = {},
               std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          inputValues(std::move(inputValues)),
          previousResponseId(std::move(previousResponseId)) {}

    LLMRequestConfig config;
    std::string prompt;
    LLMInput inputValues;
    std::string previousResponseId;  // For native OpenAI conversation management

    std::string toString() const {
        std::string inputValuesString;
        for (size_t i = 0; i < inputValues.size(); ++i) {
            if (i > 0) inputValuesString += ", ";
            inputValuesString += inputValues[i];
        }

        return "LLMRequest {\n config: " + config.toString() + ",\n prompt: " + prompt +
               ",\n inputValues: [" + inputValuesString + "]" +
               ",\n previousResponseId: " + previousResponseId + "\n}";
    }
};

struct LLMUsage {
    int inputTokens = 0;
    int outputTokens = 0;

    int totalTokens() const { return inputTokens + outputTokens; }

    std::string toString() const {
        return "LLMUsage { inputTokens: " + std::to_string(inputTokens) +
               ", outputTokens: " + std::to_string(outputTokens) +
               ", totalTokens: " + std::to_string(totalTokens()) + " }";
    }
};

struct LLMResponse {
    json result = json::object();
    bool success = false;
    std::string errorMessage;
    std::string responseId;  // For conversation continuity with OpenAI
    LLMUsage usage;          // Token usage information

    std::string toString() const {
        std::string resultString = result.dump(2);
        return "LLMResponse {\n result: " + resultString +
               ",\n success: " + (success ? "true" : "false") +
               ",\n errorMessage: " + errorMessage + ",\n responseId: " + responseId +
               ",\n usage: " + usage.toString() + "\n}";
    }
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

inline std::string toString(LLMErrorCode code) {
    switch (code) {
        case LLMErrorCode::None:
            return "None";
        case LLMErrorCode::NetworkError:
            return "NetworkError";
        case LLMErrorCode::AuthenticationError:
            return "AuthenticationError";
        case LLMErrorCode::RateLimitError:
            return "RateLimitError";
        case LLMErrorCode::InvalidRequest:
            return "InvalidRequest";
        case LLMErrorCode::ModelNotFound:
            return "ModelNotFound";
        case LLMErrorCode::InternalError:
            return "InternalError";
        case LLMErrorCode::Unknown:
            return "Unknown";
    }
    return "Unknown";
}

// Optional callback types using std::function instead of JUCE
using LLMResponseCallback = std::function<void(LLMResponse)>;
using LLMStreamCallback = std::function<void(const std::string&)>;