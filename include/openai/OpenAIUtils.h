#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * OpenAI utility functions
 * TODO: Implement actual utilities
 */
namespace OpenAIUtils {
    // URL building
    std::string buildApiUrl(const std::string& endpoint);
    
    // Request formatting
    json formatChatRequest(const std::string& prompt, const std::string& model);
    
    // Response parsing
    std::string extractResponseText(const json& response);
    
    // Error handling
    std::string parseErrorMessage(const json& errorResponse);
} 