#include "providers/ClientFactory.h"

#include <iostream>

#include "anthropic/AnthropicClient.h"
#include "openai/OpenAIClient.h"

namespace llmcpp {

std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& provider,
                                                       const json& config) {
    // Extract API key from config
    std::string apiKey;
    if (config.contains("api_key")) {
        apiKey = config["api_key"];
    } else if (config.contains("apiKey")) {
        apiKey = config["apiKey"];
    } else {
        return nullptr;  // API key is required
    }
    return createClient(provider, apiKey);
}

std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& provider,
                                                       const std::string& apiKey) {
    if (provider == "openai") {
        return std::make_unique<OpenAIClient>(apiKey);
    }

    if (provider == "anthropic") {
        return std::make_unique<Anthropic::AnthropicClient>(apiKey);
    }

    return nullptr;
}

}  // namespace llmcpp
