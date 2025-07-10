#include "providers/ClientFactory.h"
#include "openai/OpenAIClient.h"

std::unique_ptr<LLMClient> ClientFactory::createOpenAIClient(const std::string& apiKey) {
    return std::make_unique<OpenAIClient>(apiKey);
}

std::unique_ptr<LLMClient> ClientFactory::createClient(const std::string& providerName, 
                                                      const std::string& apiKey) {
    if (providerName == "openai") {
        return createOpenAIClient(apiKey);
    }
    // TODO: Add other providers
    return nullptr;
}

std::vector<std::string> ClientFactory::getAvailableProviders() {
    return {"openai"};
}

bool ClientFactory::isProviderSupported(const std::string& providerName) {
    auto providers = getAvailableProviders();
    return std::find(providers.begin(), providers.end(), providerName) != providers.end();
} 