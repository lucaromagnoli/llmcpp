#include "core/ClientFactory.h"

#include "anthropic/AnthropicClient.h"
#include "openai/OpenAIClient.h"

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

bool ClientFactory::isProviderSupported(const std::string& provider) const {
    return provider == "openai" || provider == "anthropic";
}

std::vector<std::string> ClientFactory::getSupportedProviders() const {
    return {"openai", "anthropic"};
}
