#include "openai/OpenAIClient.h"

#include <algorithm>
#include <future>
#include <stdexcept>

#include "openai/OpenAIResponsesApi.h"

// #include "openai/OpenAIChatCompletionsApi.h"
#include "openai/OpenAIHttpClient.h"

// For now, create minimal stub classes to make unique_ptr work
class OpenAIChatCompletionsApi {
   public:
    OpenAIChatCompletionsApi() = default;
    virtual ~OpenAIChatCompletionsApi() = default;
};

OpenAIClient::OpenAIClient(const std::string& apiKey)
    : config_{apiKey, "https://api.openai.com/v1"} {
    initializeApiHandlers();
}

OpenAIClient::OpenAIClient(const OpenAI::OpenAIConfig& config) : config_(config) {
    initializeApiHandlers();
}

// Destructor implementation - can handle unique_ptr destruction here
// because complete types are available (either from includes or stubs above)
OpenAIClient::~OpenAIClient() = default;

// Move constructor
OpenAIClient::OpenAIClient(OpenAIClient&& other) noexcept = default;

// Move assignment
OpenAIClient& OpenAIClient::operator=(OpenAIClient&& other) noexcept = default;

// LLMClient interface implementation
void OpenAIClient::sendRequest(const LLMRequest& request, LLMResponseCallback callback) {
    auto future = routeRequestAsync(request, callback);
    // Don't wait for the result in the async version - just fire and forget
}

void OpenAIClient::sendStreamingRequest(const LLMRequest& request, LLMResponseCallback onDone,
                                        LLMStreamCallback onChunk) {
    auto future = routeStreamingRequest(request, onChunk, onDone);
    // Don't wait for the result in the async version - just fire and forget
}

std::vector<std::string> OpenAIClient::getAvailableModels() const {
    std::vector<std::string> models;
    models.insert(models.end(), OpenAI::RESPONSES_MODELS.begin(), OpenAI::RESPONSES_MODELS.end());
    models.insert(models.end(), OpenAI::CHAT_COMPLETION_MODELS.begin(),
                  OpenAI::CHAT_COMPLETION_MODELS.end());
    return models;
}

bool OpenAIClient::supportsStreaming() const { return true; }

std::string OpenAIClient::getClientName() const { return "OpenAI"; }

// Synchronous methods
LLMResponse OpenAIClient::sendRequest(const LLMRequest& request) { return routeRequest(request); }

std::future<LLMResponse> OpenAIClient::sendRequestAsync(const LLMRequest& request,
                                                        LLMResponseCallback callback) {
    return routeRequestAsync(request, callback);
}

std::future<LLMResponse> OpenAIClient::sendStreamingRequestAsync(
    const LLMRequest& request, LLMStreamCallback streamCallback,
    LLMResponseCallback finalCallback) {
    return routeStreamingRequest(request, streamCallback, finalCallback);
}

// Configuration methods
void OpenAIClient::setApiKey(const std::string& apiKey) { config_.apiKey = apiKey; }

std::string OpenAIClient::getApiKey() const { return config_.apiKey; }

bool OpenAIClient::isConfigured() const { return !config_.apiKey.empty(); }

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
    return json{{"api_key", config_.apiKey},
                {"base_url", config_.baseUrl},
                {"organization", config_.organization},
                {"project", config_.project}};
}

// OpenAI-specific methods - now implemented using real API
OpenAI::ResponsesResponse OpenAIClient::sendResponsesRequest(
    const OpenAI::ResponsesRequest& request) {
    if (!responsesApi_) {
        throw std::runtime_error("Responses API not initialized");
    }

    std::string errorMessage;
    if (!responsesApi_->validateRequest(request, errorMessage)) {
        throw std::invalid_argument("Invalid request: " + errorMessage);
    }

    return responsesApi_->create(request);
}

std::future<OpenAI::ResponsesResponse> OpenAIClient::sendResponsesRequestAsync(
    const OpenAI::ResponsesRequest& request,
    std::function<void(const OpenAI::ResponsesResponse&)> callback) {
    if (!responsesApi_) {
        throw std::runtime_error("Responses API not initialized");
    }

    std::string errorMessage;
    if (!responsesApi_->validateRequest(request, errorMessage)) {
        throw std::invalid_argument("Invalid request: " + errorMessage);
    }

    return responsesApi_->createAsync(request, callback);
}

OpenAI::ChatCompletionResponse OpenAIClient::sendChatCompletion(
    const OpenAI::ChatCompletionRequest& request) {
    throw std::runtime_error("OpenAIClient::sendChatCompletion not yet implemented");
}

// Configuration
void OpenAIClient::setConfig(const OpenAI::OpenAIConfig& config) { config_ = config; }

OpenAI::OpenAIConfig OpenAIClient::getConfig() const { return config_; }

// API type detection and routing
OpenAI::ApiType OpenAIClient::detectApiType(const LLMRequest& request) const {
    return OpenAI::detectApiType(request);
}

void OpenAIClient::setPreferredApiType(OpenAI::ApiType apiType) { preferredApiType_ = apiType; }

OpenAI::ApiType OpenAIClient::getPreferredApiType() const { return preferredApiType_; }

// Private methods - now implemented with real routing
void OpenAIClient::initializeApiHandlers() {
    // Create HTTP client with current configuration
    httpClient_ = std::make_unique<OpenAIHttpClient>(config_);

    // Create shared pointer for API handlers
    auto sharedHttpClient =
        std::shared_ptr<OpenAIHttpClient>(httpClient_.get(), [](OpenAIHttpClient*) {});

    // Initialize API handlers with shared HTTP client
    responsesApi_ = std::make_unique<OpenAIResponsesApi>(sharedHttpClient);
    chatCompletionsApi_ = std::make_unique<OpenAIChatCompletionsApi>();
}

LLMResponse OpenAIClient::routeRequest(const LLMRequest& request) {
    try {
        // Detect which API to use
        OpenAI::ApiType apiType = detectApiType(request);

        if (preferredApiType_ != OpenAI::ApiType::AUTO_DETECT) {
            apiType = preferredApiType_;
        }

        // Route to appropriate API
        if (apiType == OpenAI::ApiType::RESPONSES || apiType == OpenAI::ApiType::AUTO_DETECT) {
            // Use Responses API (preferred for modern features)
            auto responsesRequest = OpenAI::ResponsesRequest::fromLLMRequest(request);
            auto responsesResponse = sendResponsesRequest(responsesRequest);
            return responsesResponse.toLLMResponse();
        } else if (apiType == OpenAI::ApiType::CHAT_COMPLETIONS) {
            // Chat Completions API - not yet implemented
            throw std::runtime_error("Chat Completions API not yet implemented");
        } else {
            throw std::runtime_error("Unknown API type");
        }

    } catch (const std::exception& e) {
        LLMResponse errorResponse;
        errorResponse.success = false;
        errorResponse.errorMessage = e.what();

        // Try to preserve any error details in the result
        try {
            // Check if the error message contains JSON
            std::string errorMsg = e.what();
            if (errorMsg.find("{") != std::string::npos) {
                auto start = errorMsg.find("{");
                auto jsonPart = errorMsg.substr(start);
                errorResponse.result = json::parse(jsonPart);
            }
        } catch (...) {
            // If JSON parsing fails, just keep the error message
        }

        return errorResponse;
    }
}

std::future<LLMResponse> OpenAIClient::routeRequestAsync(const LLMRequest& request,
                                                         LLMResponseCallback callback) {
    return std::async(std::launch::async, [this, request, callback]() {
        auto response = routeRequest(request);
        if (callback) {
            callback(response);
        }
        return response;
    });
}

std::future<LLMResponse> OpenAIClient::routeStreamingRequest(const LLMRequest& request,
                                                             LLMStreamCallback streamCallback,
                                                             LLMResponseCallback finalCallback) {
    return std::async(std::launch::async, [this, request, streamCallback, finalCallback]() {
        try {
            // For now, implement streaming as regular request with callback
            // Real streaming will be implemented when Responses API streaming is ready
            auto response = routeRequest(request);

            if (streamCallback && response.success) {
                // Simulate streaming by sending the complete response
                streamCallback(response.result.dump());
            }

            if (finalCallback) {
                finalCallback(response);
            }

            return response;

        } catch (const std::exception& e) {
            LLMResponse errorResponse;
            errorResponse.success = false;
            errorResponse.errorMessage = e.what();

            if (finalCallback) {
                finalCallback(errorResponse);
            }

            return errorResponse;
        }
    });
}
