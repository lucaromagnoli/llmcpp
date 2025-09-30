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

// LLMRequestConfig and LLMContext are now defined in OpenAITypes.h to avoid circular dependency

// LLMRequest is now defined in OpenAITypes.h to avoid circular dependency

// LLMUsage and LLMResponse are now defined in OpenAITypes.h to avoid circular dependency

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
