#pragma once

#include <memory>
#include <string>

#include "LLMClient.h"

/**
 * @brief Factory class for creating LLM clients
 *
 * This factory provides a unified interface for creating different types of LLM clients
 * based on provider names. It supports various providers like OpenAI, Anthropic, etc.
 */
class ClientFactory {
   public:
    ClientFactory() = default;
    ~ClientFactory() = default;

    // Disable copy semantics (use shared_ptr for sharing)
    ClientFactory(const ClientFactory&) = delete;
    ClientFactory& operator=(const ClientFactory&) = delete;

    // Enable move semantics
    ClientFactory(ClientFactory&&) = default;
    ClientFactory& operator=(ClientFactory&&) = default;

    /**
     * @brief Create a new LLM client for the specified provider
     *
     * @param provider The provider name (e.g., "openai", "anthropic")
     * @param apiKey The API key for the provider
     * @return std::unique_ptr<LLMClient> Pointer to the created client, nullptr if provider not
     * supported
     */
    std::unique_ptr<LLMClient> createClient(const std::string& provider, const std::string& apiKey);

    /**
     * @brief Check if a provider is supported
     *
     * @param provider The provider name to check
     * @return true if the provider is supported, false otherwise
     */
    bool isProviderSupported(const std::string& provider) const;

    /**
     * @brief Get list of all supported providers
     *
     * @return std::vector<std::string> List of supported provider names
     */
    std::vector<std::string> getSupportedProviders() const;
};
