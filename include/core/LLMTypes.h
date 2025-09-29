#pragma once
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// Include OpenAITypes.h to get the full definition of ToolVariant
#include "openai/OpenAITypes.h"

using json = nlohmann::json;

// Context type using standard C++ vectors of generic objects
using LLMContext = std::vector<json>;

/// Represents the configuration for the LLM
struct LLMRequestConfig {
    std::string client;
    std::string model;                          // String model name (works with any provider)
    std::string functionName = "llm_function";  // Default function name for LLM calls
    std::string jsonSchema;
    std::optional<json> schemaObject;  // Structured schema data

    std::optional<float> temperature;  // Optional temperature (filtered by model support)
    std::optional<int> maxTokens;      // Optional max tokens
    std::optional<std::vector<OpenAI::ToolVariant>> tools;  // Optional tools for function calling

    // Convenience method for any model name
    void setModel(const std::string& modelName) { model = modelName; }

    std::string getModelString() const { return model; }

    // Add more configuration options as needed (e.g., top_p, stop sequences, etc.)

    std::string toString() const {
        std::string schemaStr = schemaObject.has_value() ? schemaObject->dump() : jsonSchema;
        std::string tempStr = temperature.has_value() ? std::to_string(*temperature) : "not set";
        std::string toolsStr = tools.has_value() ? std::to_string(tools->size()) + " tools" : "no tools";
        return "LLMRequestConfig { client: " + client + ", model: " + getModelString() +
               ", functionName: " + functionName + ", schema: " + schemaStr +
               ", temperature: " + tempStr +
               ", maxTokens: " + std::to_string(maxTokens.has_value() ? *maxTokens : 0) +
               ", tools: " + toolsStr + " }";
    }
};

struct LLMRequest {
    LLMRequest() = delete;

    // Constructor with prompt only
    LLMRequest(LLMRequestConfig config, std::string prompt, LLMContext context = {},
               std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          context(std::move(context)),
          previousResponseId(std::move(previousResponseId)) {}

    // Constructor with single context object (convenience)
    LLMRequest(LLMRequestConfig config, std::string prompt, json contextObject,
               std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          context({std::move(contextObject)}),
          previousResponseId(std::move(previousResponseId)) {}

    LLMRequestConfig config;
    std::string prompt;  // The main task/prompt (what to do) - maps to instructions
    LLMContext context;  // Context data (vector of generic objects) - maps to inputValues
    std::string previousResponseId;  // For conversation continuity

    // Utility methods
    std::string instructions() const { return prompt; }  // For OpenAI mapping

    std::string toString() const {
        std::string contextString = "[";
        for (size_t i = 0; i < context.size(); ++i) {
            if (i > 0) contextString += ", ";
            contextString += context[i].dump();
        }
        contextString += "]";

        return "LLMRequest {\n config: " + config.toString() + ",\n prompt: " + prompt +
               ",\n context: " + contextString + ",\n previousResponseId: " + previousResponseId +
               "\n}";
    }
};

// LLMUsage is now defined in OpenAITypes.h to avoid circular dependency

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
