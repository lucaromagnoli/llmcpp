#include "openai/OpenAIClient.h"
#include "openai/OpenAIResponsesApi.h"
#include <stdexcept>

OpenAIClient::OpenAIClient(const std::string& apiKey)
    : config_{apiKey, "https://api.openai.com/v1"} {
    initializeApiHandlers();
}

OpenAIClient::OpenAIClient(const OpenAI::OpenAIConfig& config)
    : config_(config) {
    initializeApiHandlers();
}

// Destructor implementation needed for unique_ptr with incomplete types
OpenAIClient::~OpenAIClient() {
    // Clean up raw pointers (when they are actually implemented)
    delete chatCompletionsApi_;
    delete completionsApi_;
    delete httpClient_;
    // responsesApi_ is handled by unique_ptr automatically
}

// LLMClient interface implementation
void OpenAIClient::sendRequest(const LLMRequest& request, LLMResponseCallback callback) {
    // TODO: Implement async request
    throw std::runtime_error("OpenAIClient::sendRequest not yet implemented");
}

void OpenAIClient::sendStreamingRequest(const LLMRequest& request, 
                                       LLMResponseCallback onDone,
                                       LLMStreamCallback onChunk) {
    // TODO: Implement streaming request
    throw std::runtime_error("OpenAIClient::sendStreamingRequest not yet implemented");
}

std::vector<std::string> OpenAIClient::getAvailableModels() const {
    std::vector<std::string> models;
    models.insert(models.end(), OpenAI::RESPONSES_MODELS.begin(), OpenAI::RESPONSES_MODELS.end());
    models.insert(models.end(), OpenAI::CHAT_COMPLETION_MODELS.begin(), OpenAI::CHAT_COMPLETION_MODELS.end());
    models.insert(models.end(), OpenAI::COMPLETION_MODELS.begin(), OpenAI::COMPLETION_MODELS.end());
    return models;
}

bool OpenAIClient::supportsStreaming() const {
    return true;
}

std::string OpenAIClient::getClientName() const {
    return "OpenAI";
}

// Synchronous methods
LLMResponse OpenAIClient::sendRequest(const LLMRequest& request) {
    return routeRequest(request);
}

std::future<LLMResponse> OpenAIClient::sendRequestAsync(const LLMRequest& request,
                                                       LLMResponseCallback callback) {
    return routeRequestAsync(request, callback);
}

std::future<LLMResponse> OpenAIClient::sendStreamingRequestAsync(const LLMRequest& request,
                                                                LLMStreamCallback streamCallback,
                                                                LLMResponseCallback finalCallback) {
    return routeStreamingRequest(request, streamCallback, finalCallback);
}

// Configuration methods
void OpenAIClient::setApiKey(const std::string& apiKey) {
    config_.apiKey = apiKey;
}

std::string OpenAIClient::getApiKey() const {
    return config_.apiKey;
}

bool OpenAIClient::isConfigured() const {
    return !config_.apiKey.empty();
}

bool OpenAIClient::isModelSupported(const std::string& modelName) const {
    auto models = getAvailableModels();
    return std::find(models.begin(), models.end(), modelName) != models.end();
}

void OpenAIClient::setClientConfig(const json& config) {
    if (config.contains("api_key")) {
        config_.apiKey = config["api_key"].get<std::string>();
    }
    if (config.contains("base_url")) {
        config_.baseUrl = config["base_url"].get<std::string>();
    }
    if (config.contains("organization")) {
        config_.organization = config["organization"].get<std::string>();
    }
    if (config.contains("project")) {
        config_.project = config["project"].get<std::string>();
    }
}

json OpenAIClient::getClientConfig() const {
    return json{
        {"api_key", config_.apiKey},
        {"base_url", config_.baseUrl},
        {"organization", config_.organization},
        {"project", config_.project}
    };
}

// OpenAI-specific methods (stubs)
OpenAI::ResponsesResponse OpenAIClient::sendResponsesRequest(const OpenAI::ResponsesRequest& request) {
    throw std::runtime_error("OpenAIClient::sendResponsesRequest not yet implemented");
}

std::future<OpenAI::ResponsesResponse> OpenAIClient::sendResponsesRequestAsync(
    const OpenAI::ResponsesRequest& request,
    std::function<void(const OpenAI::ResponsesResponse&)> callback) {
    throw std::runtime_error("OpenAIClient::sendResponsesRequestAsync not yet implemented");
}

OpenAI::ChatCompletionResponse OpenAIClient::sendChatCompletion(const OpenAI::ChatCompletionRequest& request) {
    throw std::runtime_error("OpenAIClient::sendChatCompletion not yet implemented");
}

OpenAI::CompletionResponse OpenAIClient::sendCompletion(const OpenAI::CompletionRequest& request) {
    throw std::runtime_error("OpenAIClient::sendCompletion not yet implemented");
}

// Configuration
void OpenAIClient::setConfig(const OpenAI::OpenAIConfig& config) {
    config_ = config;
}

OpenAI::OpenAIConfig OpenAIClient::getConfig() const {
    return config_;
}

// API type detection and routing
OpenAI::ApiType OpenAIClient::detectApiType(const LLMRequest& request) const {
    return OpenAI::detectApiType(request);
}

void OpenAIClient::setPreferredApiType(OpenAI::ApiType apiType) {
    preferredApiType_ = apiType;
}

OpenAI::ApiType OpenAIClient::getPreferredApiType() const {
    return preferredApiType_;
}

// Private methods (stubs)
void OpenAIClient::initializeApiHandlers() {
    // TODO: Initialize actual API handlers when they are implemented
    responsesApi_ = std::make_unique<OpenAIResponsesApi>(nullptr);
    // chatCompletionsApi_ and completionsApi_ will be initialized as nullptr for now
    chatCompletionsApi_ = nullptr;
    completionsApi_ = nullptr;
    httpClient_ = nullptr;
}

LLMResponse OpenAIClient::routeRequest(const LLMRequest& request) {
    throw std::runtime_error("OpenAIClient::routeRequest not yet implemented");
}

std::future<LLMResponse> OpenAIClient::routeRequestAsync(const LLMRequest& request, 
                                                        LLMResponseCallback callback) {
    throw std::runtime_error("OpenAIClient::routeRequestAsync not yet implemented");
}

std::future<LLMResponse> OpenAIClient::routeStreamingRequest(const LLMRequest& request,
                                                            LLMStreamCallback streamCallback,
                                                            LLMResponseCallback finalCallback) {
    throw std::runtime_error("OpenAIClient::routeStreamingRequest not yet implemented");
} 