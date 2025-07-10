#include "openai/OpenAIUtils.h"

namespace OpenAIUtils {
    std::string buildApiUrl(const std::string& endpoint) {
        return "https://api.openai.com/v1/" + endpoint;
    }
    
    json formatChatRequest(const std::string& prompt, const std::string& model) {
        // Stub implementation
        return json{
            {"model", model},
            {"messages", json::array({
                {{"role", "user"}, {"content", prompt}}
            })}
        };
    }
    
    std::string extractResponseText(const json& response) {
        // Stub implementation
        return "stub response";
    }
    
    std::string parseErrorMessage(const json& errorResponse) {
        // Stub implementation
        return "unknown error";
    }
} 