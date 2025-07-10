#include "openai/OpenAIClient.h"
#include "openai/OpenAIModels.h"

OpenAIClient::OpenAIClient(const std::string& apiKey) : apiKey_(apiKey) {
    // Constructor stub
}

LLMResponse OpenAIClient::sendRequest(const LLMRequest& request) {
    LLMResponse response;
    response.success = false;
    response.errorMessage = "OpenAI client not implemented yet";
    return response;
}

std::future<LLMResponse> OpenAIClient::sendRequestAsync(const LLMRequest& request,
                                                       LLMResponseCallback callback) {
    return std::async(std::launch::async, [this, request, callback]() {
        auto response = sendRequest(request);
        if (callback) {
            callback(response);
        }
        return response;
    });
}

std::future<LLMResponse> OpenAIClient::sendStreamingRequest(const LLMRequest& request,
                                                           LLMStreamCallback streamCallback,
                                                           LLMResponseCallback finalCallback) {
    return std::async(std::launch::async, [this, request, streamCallback, finalCallback]() {
        auto response = sendRequest(request);
        if (finalCallback) {
            finalCallback(response);
        }
        return response;
    });
}

void OpenAIClient::setApiKey(const std::string& apiKey) {
    apiKey_ = apiKey;
}

std::string OpenAIClient::getApiKey() const {
    return apiKey_.empty() ? "" : "***masked***";
}

bool OpenAIClient::isConfigured() const {
    return !apiKey_.empty();
}

std::string OpenAIClient::getClientName() const {
    return "openai";
}

std::vector<std::string> OpenAIClient::getAvailableModels() const {
    return OpenAIModels::getAllModels();
}

bool OpenAIClient::isModelSupported(const std::string& modelName) const {
    auto models = getAvailableModels();
    return std::find(models.begin(), models.end(), modelName) != models.end();
}

void OpenAIClient::setClientConfig(const json& config) {
    clientConfig_ = config;
}

json OpenAIClient::getClientConfig() const {
    return clientConfig_;
} 