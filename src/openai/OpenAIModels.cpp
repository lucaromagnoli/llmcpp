#include "openai/OpenAIModels.h"

namespace OpenAIModels {
    // Model constants
    const std::string GPT_4 = "gpt-4";
    const std::string GPT_4_TURBO = "gpt-4-turbo";
    const std::string GPT_4O = "gpt-4o";
    const std::string GPT_4O_MINI = "gpt-4o-mini";
    const std::string GPT_3_5_TURBO = "gpt-3.5-turbo";
    
    std::vector<std::string> getAllModels() {
        return {GPT_4, GPT_4_TURBO, GPT_4O, GPT_4O_MINI, GPT_3_5_TURBO};
    }
    
    bool supportsTools(const std::string& model) {
        // Stub implementation
        return true;
    }
    
    bool supportsJsonMode(const std::string& model) {
        // Stub implementation
        return true;
    }
    
    int getContextWindow(const std::string& model) {
        // Stub implementation
        return 8192;
    }
} 