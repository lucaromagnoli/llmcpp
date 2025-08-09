#include "anthropic/AnthropicClient.h"

#include <future>
#include <stdexcept>
#include <thread>

#include "anthropic/AnthropicHttpClient.h"

namespace Anthropic {

/**
 * PIMPL implementation for AnthropicClient
 */
class AnthropicClient::ClientImpl {
   public:
    explicit ClientImpl(const AnthropicConfig& config)
        : config_(config), httpClient_(std::make_unique<AnthropicHttpClient>(config)) {}

    void sendRequest(const LLMRequest& request, LLMResponseCallback callback) {
        // Run the request in a separate thread to make it async
        std::thread([this, request, callback]() {
            try {
                auto response = sendRequestSync(request);
                callback(response);
            } catch (const std::exception& e) {
                LLMResponse errorResponse;
                errorResponse.success = false;
                errorResponse.errorMessage = e.what();
                callback(errorResponse);
            }
        }).detach();
    }

    void sendStreamingRequest(const LLMRequest& request, LLMResponseCallback onDone,
                              LLMStreamCallback onChunk) {
        // Anthropic doesn't support streaming in this implementation yet
        // Fall back to regular request
        (void)onChunk;  // Suppress unused parameter warning
        sendRequest(request, std::move(onDone));
    }

    LLMResponse sendRequestSync(const LLMRequest& request) {
        try {
            // Convert LLMRequest to MessagesRequest
            auto messagesRequest = MessagesRequest::fromLLMRequest(request);

            // Use default model if none specified
            if (messagesRequest.model.empty()) {
                messagesRequest.model = toString(config_.defaultModel);
            }

            // Send the request
            auto messagesResponse = httpClient_->sendMessagesRequest(messagesRequest);

            // Convert back to LLMResponse
            return messagesResponse.toLLMResponse();
        } catch (const std::exception& e) {
            LLMResponse errorResponse;
            errorResponse.success = false;
            errorResponse.errorMessage = e.what();
            return errorResponse;
        }
    }

    MessagesResponse sendMessagesRequest(const MessagesRequest& request) {
        return httpClient_->sendMessagesRequest(request);
    }

    std::vector<std::string> getAvailableModels() const { return Anthropic::getAvailableModels(); }

    bool supportsStreaming() const {
        return false;  // Not implemented yet
    }

    std::string getClientName() const { return "AnthropicClient"; }

    const AnthropicConfig& getConfig() const { return config_; }

    void setApiKey(const std::string& apiKey) {
        config_.apiKey = apiKey;
        // Recreate HTTP client with new config
        httpClient_ = std::make_unique<AnthropicHttpClient>(config_);
    }

    void setDefaultModel(Model model) { config_.defaultModel = model; }

   private:
    AnthropicConfig config_;
    std::unique_ptr<AnthropicHttpClient> httpClient_;
};

// AnthropicClient implementation
AnthropicClient::AnthropicClient(const std::string& apiKey)
    : pImpl(std::make_unique<ClientImpl>(AnthropicConfig(apiKey))) {}

AnthropicClient::AnthropicClient(const AnthropicConfig& config)
    : pImpl(std::make_unique<ClientImpl>(config)) {}

AnthropicClient::AnthropicClient(const std::string& apiKey, Model defaultModel) {
    AnthropicConfig config(apiKey);
    config.defaultModel = defaultModel;
    pImpl = std::make_unique<ClientImpl>(config);
}

AnthropicClient::~AnthropicClient() = default;

AnthropicClient::AnthropicClient(AnthropicClient&& other) noexcept = default;
AnthropicClient& AnthropicClient::operator=(AnthropicClient&& other) noexcept = default;

void AnthropicClient::sendRequest(const LLMRequest& request, LLMResponseCallback callback) {
    pImpl->sendRequest(request, std::move(callback));
}

void AnthropicClient::sendStreamingRequest(const LLMRequest& request, LLMResponseCallback onDone,
                                           LLMStreamCallback onChunk) {
    pImpl->sendStreamingRequest(request, std::move(onDone), std::move(onChunk));
}

std::vector<std::string> AnthropicClient::getAvailableModels() const {
    return pImpl->getAvailableModels();
}

bool AnthropicClient::supportsStreaming() const { return pImpl->supportsStreaming(); }

std::string AnthropicClient::getClientName() const { return pImpl->getClientName(); }

MessagesResponse AnthropicClient::sendMessagesRequest(const MessagesRequest& request) {
    return pImpl->sendMessagesRequest(request);
}

LLMResponse AnthropicClient::sendRequest(const LLMRequest& request) {
    return pImpl->sendRequestSync(request);
}

const AnthropicConfig& AnthropicClient::getConfig() const { return pImpl->getConfig(); }

void AnthropicClient::setApiKey(const std::string& apiKey) { pImpl->setApiKey(apiKey); }

void AnthropicClient::setDefaultModel(Model model) { pImpl->setDefaultModel(model); }

}  // namespace Anthropic
