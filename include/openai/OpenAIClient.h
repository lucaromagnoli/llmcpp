#pragma once
#include "core/LLMClient.h"

/**
 * OpenAI client implementation
 * TODO: Implement full OpenAI API client
 */
class OpenAIClient : public LLMClient {
public:
    explicit OpenAIClient(const std::string& apiKey = "");
    
    // LLMClient interface
    LLMResponse sendRequest(const LLMRequest& request) override;
    std::future<LLMResponse> sendRequestAsync(const LLMRequest& request,
                                             LLMResponseCallback callback = nullptr) override;
    std::future<LLMResponse> sendStreamingRequest(const LLMRequest& request,
                                                 LLMStreamCallback streamCallback,
                                                 LLMResponseCallback finalCallback = nullptr) override;
    
    void setApiKey(const std::string& apiKey) override;
    std::string getApiKey() const override;
    bool isConfigured() const override;
    std::string getClientName() const override;
    std::vector<std::string> getAvailableModels() const override;
    bool isModelSupported(const std::string& modelName) const override;
    void setClientConfig(const json& config) override;
    json getClientConfig() const override;

private:
    std::string apiKey_;
    json clientConfig_;
}; 