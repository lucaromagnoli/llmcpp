#pragma once
#include "core/LLMClient.h"
#include "openai/OpenAITypes.h"
#include "openai/OpenAIResponsesApi.h"
#include <memory>
#include <functional>
#include <future>
#include <string>
#include <vector>

// Forward declarations
class OpenAIChatCompletionsApi;
class OpenAICompletionsApi;
class OpenAIHttpClient;

/**
 * OpenAI client implementation supporting all three APIs: Responses, Chat Completions, and Completions
 */
class OpenAIClient : public LLMClient {
public:
    explicit OpenAIClient(const std::string& apiKey = "");
    explicit OpenAIClient(const OpenAI::OpenAIConfig& config);
    ~OpenAIClient(); // Need explicit destructor for unique_ptr with incomplete types
    
    /**
     * LLMClient interface implementation
     */
    void sendRequest(const LLMRequest& request, LLMResponseCallback callback) override;
    void sendStreamingRequest(const LLMRequest& request, 
                            LLMResponseCallback onDone,
                            LLMStreamCallback onChunk) override;
    std::vector<std::string> getAvailableModels() const override;
    bool supportsStreaming() const override;
    std::string getClientName() const override;
    
    /**
     * Synchronous methods (convenience)
     */
    LLMResponse sendRequest(const LLMRequest& request);
    std::future<LLMResponse> sendRequestAsync(const LLMRequest& request,
                                             LLMResponseCallback callback = nullptr);
    std::future<LLMResponse> sendStreamingRequestAsync(const LLMRequest& request,
                                                      LLMStreamCallback streamCallback,
                                                      LLMResponseCallback finalCallback = nullptr);
    
    /**
     * Configuration methods
     */
    void setApiKey(const std::string& apiKey);
    std::string getApiKey() const;
    bool isConfigured() const;
    bool isModelSupported(const std::string& modelName) const;
    void setClientConfig(const json& config);
    json getClientConfig() const;
    
    /**
     * OpenAI-specific methods
     */
    
    // Responses API (Modern Structured Output - Primary API)
    OpenAI::ResponsesResponse sendResponsesRequest(const OpenAI::ResponsesRequest& request);
    std::future<OpenAI::ResponsesResponse> sendResponsesRequestAsync(
        const OpenAI::ResponsesRequest& request,
        std::function<void(const OpenAI::ResponsesResponse&)> callback = nullptr);
    std::future<OpenAI::ResponsesResponse> sendResponsesStreaming(
        const OpenAI::ResponsesRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::ResponsesResponse&)> finalCallback = nullptr);
    
    // Background task management for Responses API
    OpenAI::ResponsesResponse retrieveResponse(const std::string& responseId);
    OpenAI::ResponsesResponse cancelResponse(const std::string& responseId);
    OpenAI::ResponsesResponse deleteResponse(const std::string& responseId);
    
    // Chat Completions API (Traditional Conversational)
    OpenAI::ChatCompletionResponse sendChatCompletion(const OpenAI::ChatCompletionRequest& request);
    std::future<OpenAI::ChatCompletionResponse> sendChatCompletionAsync(
        const OpenAI::ChatCompletionRequest& request,
        std::function<void(const OpenAI::ChatCompletionResponse&)> callback = nullptr);
    std::future<OpenAI::ChatCompletionResponse> sendChatCompletionStreaming(
        const OpenAI::ChatCompletionRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::ChatCompletionResponse&)> finalCallback = nullptr);
    
    // Completions API (Legacy - with deprecation warnings)
    OpenAI::CompletionResponse sendCompletion(const OpenAI::CompletionRequest& request);
    std::future<OpenAI::CompletionResponse> sendCompletionAsync(
        const OpenAI::CompletionRequest& request,
        std::function<void(const OpenAI::CompletionResponse&)> callback = nullptr);
    std::future<OpenAI::CompletionResponse> sendCompletionStreaming(
        const OpenAI::CompletionRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::CompletionResponse&)> finalCallback = nullptr);
    
    // Configuration
    void setConfig(const OpenAI::OpenAIConfig& config);
    OpenAI::OpenAIConfig getConfig() const;
    
    // API type detection and routing
    OpenAI::ApiType detectApiType(const LLMRequest& request) const;
    void setPreferredApiType(OpenAI::ApiType apiType);
    OpenAI::ApiType getPreferredApiType() const;
    
    // Deprecation warnings
    void enableDeprecationWarnings(bool enable);
    bool areDeprecationWarningsEnabled() const;
    
    // Model migration helpers
    std::string getRecommendedModel(const std::string& currentModel) const;
    std::vector<std::string> getModelsForApiType(OpenAI::ApiType apiType) const;
    
private:
    /**
     * Internal API handlers
     */
    std::unique_ptr<OpenAIResponsesApi> responsesApi_;
    OpenAIChatCompletionsApi* chatCompletionsApi_;
    OpenAICompletionsApi* completionsApi_;
    OpenAIHttpClient* httpClient_;
    
    /**
     * Configuration
     */
    OpenAI::OpenAIConfig config_;
    OpenAI::ApiType preferredApiType_ = OpenAI::ApiType::AUTO_DETECT;
    
    /**
     * Internal routing methods
     */
    LLMResponse routeRequest(const LLMRequest& request);
    std::future<LLMResponse> routeRequestAsync(const LLMRequest& request, 
                                              LLMResponseCallback callback);
    std::future<LLMResponse> routeStreamingRequest(const LLMRequest& request,
                                                  LLMStreamCallback streamCallback,
                                                  LLMResponseCallback finalCallback);
    
    /**
     * Helper methods
     */
    void initializeApiHandlers();
    void logDeprecationWarning(const std::string& model, const std::string& api) const;
    bool shouldUseResponsesApi(const LLMRequest& request) const;
    bool shouldUseChatCompletionsApi(const LLMRequest& request) const;
    bool shouldUseCompletionsApi(const LLMRequest& request) const;
    OpenAI::ResponsesRequest convertToResponses(const LLMRequest& request) const;
    OpenAI::ChatCompletionRequest convertToChat(const LLMRequest& request) const;
    OpenAI::CompletionRequest convertToCompletion(const LLMRequest& request) const;
    
    /**
     * Validation
     */
    void validateRequest(const LLMRequest& request) const;
    void validateResponsesRequest(const OpenAI::ResponsesRequest& request) const;
    void validateChatCompletionRequest(const OpenAI::ChatCompletionRequest& request) const;
    void validateCompletionRequest(const OpenAI::CompletionRequest& request) const;
}; 