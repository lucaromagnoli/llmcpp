#pragma once
#include "core/LLMClient.h"
#include <memory>
#include <string>

/**
 * Factory for creating LLM clients
 * TODO: Implement full factory pattern
 */
class ClientFactory {
public:
    // Create OpenAI client
    static std::unique_ptr<LLMClient> createOpenAIClient(const std::string& apiKey = "");
    
    // Create client by name
    static std::unique_ptr<LLMClient> createClient(const std::string& providerName, 
                                                  const std::string& apiKey = "");
    
    // Get list of available providers
    static std::vector<std::string> getAvailableProviders();
    
    // Check if provider is supported
    static bool isProviderSupported(const std::string& providerName);
}; 