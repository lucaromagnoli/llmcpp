#pragma once
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

using json = nlohmann::json;

// Forward declaration for OpenAI model enum
namespace OpenAI {
enum class Model;
std::string toString(Model model);
Model modelFromString(const std::string& modelStr);
}  // namespace OpenAI

// Input type using standard C++ vectors instead of JUCE StringArray
using LLMInput = std::vector<std::string>;

/**
 * @brief Common schema names for structured outputs
 *
 * Provides type-safe schema naming to prevent typos and improve maintainability.
 * These correspond to common use cases in AI applications.
 */
enum class ResponsesSchemaName {
    SentimentAnalysis,  // sentiment_analysis
    DataExtraction,     // data_extraction
    Classification,     // classification
    EntityExtraction,   // entity_extraction
    ContentAnalysis,    // content_analysis
    DocumentAnalysis,   // document_analysis
    ReviewAnalysis,     // review_analysis
    WeatherInfo,        // weather_info
    ProductInfo,        // product_info
    UserProfile,        // user_profile
    ImageAnalysis,      // image_analysis
    TextSummary,        // text_summary
    Translation,        // translation
    MathSolution,       // math_solution
    CodeAnalysis,       // code_analysis
    Custom              // For user-defined schema names
};

/**
 * @brief Convert ResponsesSchemaName enum to string
 */
inline std::string toString(ResponsesSchemaName schemaName) {
    switch (schemaName) {
        case ResponsesSchemaName::SentimentAnalysis:
            return "sentiment_analysis";
        case ResponsesSchemaName::DataExtraction:
            return "data_extraction";
        case ResponsesSchemaName::Classification:
            return "classification";
        case ResponsesSchemaName::EntityExtraction:
            return "entity_extraction";
        case ResponsesSchemaName::ContentAnalysis:
            return "content_analysis";
        case ResponsesSchemaName::DocumentAnalysis:
            return "document_analysis";
        case ResponsesSchemaName::ReviewAnalysis:
            return "review_analysis";
        case ResponsesSchemaName::WeatherInfo:
            return "weather_info";
        case ResponsesSchemaName::ProductInfo:
            return "product_info";
        case ResponsesSchemaName::UserProfile:
            return "user_profile";
        case ResponsesSchemaName::ImageAnalysis:
            return "image_analysis";
        case ResponsesSchemaName::TextSummary:
            return "text_summary";
        case ResponsesSchemaName::Translation:
            return "translation";
        case ResponsesSchemaName::MathSolution:
            return "math_solution";
        case ResponsesSchemaName::CodeAnalysis:
            return "code_analysis";
        case ResponsesSchemaName::Custom:
            return "custom_schema";
    }
    return "unknown_schema";
}

/// Represents the configuration for the LLM
struct LLMRequestConfig {
    std::string client;
    std::string model;  // String model name (for backward compatibility and non-OpenAI providers)
    std::optional<OpenAI::Model> openaiModel;   // OpenAI-specific model enum (preferred for OpenAI)
    std::string functionName = "llm_function";  // Default function name for LLM calls
    std::string jsonSchema;
    std::optional<json> schemaObject;  // Structured schema data

    float temperature = 0.8f;
    int maxTokens = 200;

    // Convenience methods for OpenAI model enum
    void setModel(OpenAI::Model openaiModelEnum) {
        openaiModel = openaiModelEnum;
        model = OpenAI::toString(openaiModelEnum);
    }

    void setModel(const std::string& modelName) {
        model = modelName;
        if (client == "openai") {
            openaiModel = OpenAI::modelFromString(modelName);
        }
    }

    std::string getModelString() const {
        if (openaiModel.has_value()) {
            return OpenAI::toString(openaiModel.value());
        }
        return model;
    }

    // Add more configuration options as needed (e.g., top_p, stop sequences, etc.)

    std::string toString() const {
        std::string schemaStr = schemaObject.has_value() ? schemaObject->dump() : jsonSchema;
        return "LLMRequestConfig { client: " + client + ", model: " + getModelString() +
               ", functionName: " + functionName + ", schema: " + schemaStr +
               ", temperature: " + std::to_string(temperature) +
               ", maxTokens: " + std::to_string(maxTokens) + " }";
    }
};

struct LLMRequest {
    LLMRequest() = delete;

    // Constructor with prompt only
    LLMRequest(LLMRequestConfig config, std::string prompt, LLMInput inputValues = {},
               std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          inputValues(std::move(inputValues)),
          previousResponseId(std::move(previousResponseId)) {}

    // Constructor with single context message (convenience)
    LLMRequest(LLMRequestConfig config, std::string prompt, std::string contextMessage,
               LLMInput inputValues = {}, std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          contextData(std::move(contextMessage)),
          inputValues(std::move(inputValues)),
          previousResponseId(std::move(previousResponseId)) {}

    // Constructor with structured context (provider-specific)
    LLMRequest(LLMRequestConfig config, std::string prompt, json contextData,
               LLMInput inputValues = {}, std::string previousResponseId = "")
        : config(std::move(config)),
          prompt(std::move(prompt)),
          contextData(std::move(contextData)),
          inputValues(std::move(inputValues)),
          previousResponseId(std::move(previousResponseId)) {}

    LLMRequestConfig config;
    std::string prompt;              // The main task/prompt (what to do)
    json contextData;                // Context data (provider-specific format)
    LLMInput inputValues;            // Template variables for substitution
    std::string previousResponseId;  // For conversation continuity

    // Utility methods
    std::string instructions() const { return prompt; }  // For OpenAI mapping
    std::string context() const {
        // Return context as string if it's a string, empty otherwise
        if (contextData.is_string()) {
            return contextData.get<std::string>();
        }
        return "";
    }

    std::string toString() const {
        std::string inputValuesString;
        for (size_t i = 0; i < inputValues.size(); ++i) {
            if (i > 0) inputValuesString += ", ";
            inputValuesString += inputValues[i];
        }

        std::string contextString = contextData.dump();

        return "LLMRequest {\n config: " + config.toString() + ",\n prompt: " + prompt +
               ",\n contextData: " + contextString + ",\n inputValues: [" + inputValuesString +
               "]" + ",\n previousResponseId: " + previousResponseId + "\n}";
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
