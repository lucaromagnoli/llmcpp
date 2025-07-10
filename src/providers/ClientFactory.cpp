#include "providers/ClientFactory.h"
#include "openai/OpenAIClient.h"
#include <stdexcept>

namespace llmcpp {

std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& provider, const json& config) {
    if (provider == "openai") {
        std::string apiKey = config.value("api_key", "");
        if (apiKey.empty()) {
            throw std::runtime_error("OpenAI API key is required");
        }
        return std::make_unique<OpenAIClient>(apiKey);
    }
    
    throw std::runtime_error("Unsupported provider: " + provider);
}

} // namespace llmcpp 