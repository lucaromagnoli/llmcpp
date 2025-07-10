#include "core/LLMTypes.h"

// Implementation file for LLMTypes
// Most types are header-only, but we can add utility functions here if needed

namespace LLMTypeUtils {
    LLMErrorCode stringToErrorCode(const juce::String& error) {
        if (error.containsIgnoreCase("network") || error.containsIgnoreCase("connection"))
            return LLMErrorCode::NetworkError;
        if (error.containsIgnoreCase("auth") || error.containsIgnoreCase("unauthorized"))
            return LLMErrorCode::AuthenticationError;
        if (error.containsIgnoreCase("rate") || error.containsIgnoreCase("limit"))
            return LLMErrorCode::RateLimitError;
        if (error.containsIgnoreCase("invalid") || error.containsIgnoreCase("bad request"))
            return LLMErrorCode::InvalidRequest;
        if (error.containsIgnoreCase("model") || error.containsIgnoreCase("not found"))
            return LLMErrorCode::ModelNotFound;
        if (error.containsIgnoreCase("internal") || error.containsIgnoreCase("server"))
            return LLMErrorCode::InternalError;
        
        return LLMErrorCode::Unknown;
    }
    
    juce::String errorCodeToString(LLMErrorCode code) {
        switch (code) {
            case LLMErrorCode::None: return "None";
            case LLMErrorCode::NetworkError: return "Network Error";
            case LLMErrorCode::AuthenticationError: return "Authentication Error";
            case LLMErrorCode::RateLimitError: return "Rate Limit Error";
            case LLMErrorCode::InvalidRequest: return "Invalid Request";
            case LLMErrorCode::ModelNotFound: return "Model Not Found";
            case LLMErrorCode::InternalError: return "Internal Error";
            case LLMErrorCode::Unknown: return "Unknown Error";
        }
        return "Unknown";
    }
} 