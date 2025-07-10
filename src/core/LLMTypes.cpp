#include "core/LLMTypes.h"
#include <algorithm>
#include <cctype>
#include <string>

// Implementation file for LLMTypes
// Most types are header-only, but we can add utility functions here if needed

namespace LLMTypeUtils {
    // Helper function to check if string contains substring (case-insensitive)
    bool containsIgnoreCase(const std::string& str, const std::string& substr) {
        std::string strLower = str;
        std::string substrLower = substr;
        
        std::transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
        std::transform(substrLower.begin(), substrLower.end(), substrLower.begin(), ::tolower);
        
        return strLower.find(substrLower) != std::string::npos;
    }
    
    LLMErrorCode stringToErrorCode(const std::string& error) {
        if (containsIgnoreCase(error, "network") || containsIgnoreCase(error, "connection"))
            return LLMErrorCode::NetworkError;
        if (containsIgnoreCase(error, "auth") || containsIgnoreCase(error, "unauthorized"))
            return LLMErrorCode::AuthenticationError;
        if (containsIgnoreCase(error, "rate") || containsIgnoreCase(error, "limit"))
            return LLMErrorCode::RateLimitError;
        if (containsIgnoreCase(error, "invalid") || containsIgnoreCase(error, "bad request"))
            return LLMErrorCode::InvalidRequest;
        if (containsIgnoreCase(error, "model") || containsIgnoreCase(error, "not found"))
            return LLMErrorCode::ModelNotFound;
        if (containsIgnoreCase(error, "internal") || containsIgnoreCase(error, "server"))
            return LLMErrorCode::InternalError;
        
        return LLMErrorCode::Unknown;
    }
    
    std::string errorCodeToString(LLMErrorCode code) {
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