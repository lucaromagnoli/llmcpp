#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "anthropic/AnthropicTypes.h"
#include "core/LLMClient.h"

// Forward declarations
class AnthropicHttpClient;

namespace Anthropic {

/**
 * Anthropic client implementation
 * Supports Claude models via Messages API
 */
class AnthropicClient : public LLMClient {
   public:
    /**
     * Constructors
     */
    explicit AnthropicClient(const std::string& apiKey);
    explicit AnthropicClient(const AnthropicConfig& config);

    // Convenience constructor with Model enum
    AnthropicClient(const std::string& apiKey, Model defaultModel);

    // Destructor
    ~AnthropicClient() override;

    // Move semantics
    AnthropicClient(AnthropicClient&& other) noexcept;
    AnthropicClient& operator=(AnthropicClient&& other) noexcept;

    // Delete copy constructor and assignment
    AnthropicClient(const AnthropicClient&) = delete;
    AnthropicClient& operator=(const AnthropicClient&) = delete;

    /**
     * LLMClient interface implementation
     */
    void sendRequest(const LLMRequest& request, LLMResponseCallback callback) override;
    void sendStreamingRequest(const LLMRequest& request, LLMResponseCallback onDone,
                              LLMStreamCallback onChunk) override;
    std::vector<std::string> getAvailableModels() const override;
    bool supportsStreaming() const override;
    std::string getClientName() const override;

    /**
     * Anthropic-specific methods
     */

    /**
     * Send a Messages API request directly
     */
    MessagesResponse sendMessagesRequest(const MessagesRequest& request);

    /**
     * Synchronous request (blocking)
     */
    LLMResponse sendRequest(const LLMRequest& request);

    /**
     * Get the current configuration
     */
    const AnthropicConfig& getConfig() const;

    /**
     * Update the API key
     */
    void setApiKey(const std::string& apiKey);

    /**
     * Update the default model
     */
    void setDefaultModel(Model model);

   private:
    class ClientImpl;
    std::unique_ptr<ClientImpl> pImpl;
};

}  // namespace Anthropic
