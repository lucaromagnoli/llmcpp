#pragma once
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "core/LLMClient.h"
#include "openai/OpenAIResponsesApi.h"
#include "openai/OpenAITypes.h"

// Forward declarations
class OpenAIChatCompletionsApi;
class OpenAIHttpClient;

/**
 * OpenAI client implementation
 * Supports both Responses API (modern) and Chat Completions API (traditional)
 */
class OpenAIClient : public LLMClient {
   public:
    /**
     * Constructors
     */
    OpenAIClient(const std::string& apiKey);
    OpenAIClient(const OpenAI::OpenAIConfig& config);

    // Convenience constructor with Model enum
    OpenAIClient(const std::string& apiKey, OpenAI::Model defaultModel);

    // Destructor
    ~OpenAIClient() override;

    // Move semantics
    OpenAIClient(OpenAIClient&& other) noexcept;
    OpenAIClient& operator=(OpenAIClient&& other) noexcept;

    // Delete copy constructor and assignment
    OpenAIClient(const OpenAIClient&) = delete;
    OpenAIClient& operator=(const OpenAIClient&) = delete;

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
     * Synchronous methods (convenience)
     */
    LLMResponse sendRequest(const LLMRequest& request);
    std::future<LLMResponse> sendRequestAsync(const LLMRequest& request,
                                              LLMResponseCallback callback = nullptr);
    std::future<LLMResponse> sendStreamingRequestAsync(const LLMRequest& request,
                                                       LLMStreamCallback streamCallback,
                                                       LLMResponseCallback finalCallback = nullptr);

    /**
     * Convenience methods with Model enum
     */
    LLMResponse sendRequest(OpenAI::Model model, const std::string& prompt, LLMContext context = {},
                            int maxTokens = 200, float temperature = 0.7f);
    std::future<LLMResponse> sendRequestAsync(OpenAI::Model model, const std::string& prompt,
                                              LLMResponseCallback callback = nullptr,
                                              LLMContext context = {}, int maxTokens = 200,
                                              float temperature = 0.7f);

    /**
     * Configuration methods
     */
    void setApiKey(const std::string& apiKey);
    std::string getApiKey() const;
    bool isConfigured() const;
    bool isModelSupported(const std::string& modelName) const;
    bool isModelSupported(OpenAI::Model model) const;
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

    // Model enum helpers
    static std::string modelToString(OpenAI::Model model);
    static OpenAI::Model stringToModel(const std::string& modelStr);
    static std::vector<OpenAI::Model> getAvailableModelEnums();

   private:
    /**
     * Internal API handlers - using unique_ptr with Pimpl idiom
     * Destruction handled in .cpp where complete types are available
     */
    std::unique_ptr<OpenAIResponsesApi> responsesApi_;
    std::unique_ptr<OpenAIChatCompletionsApi> chatCompletionsApi_;
    std::unique_ptr<OpenAIHttpClient> httpClient_;

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
    OpenAI::ResponsesRequest convertToResponses(const LLMRequest& request) const;
    OpenAI::ChatCompletionRequest convertToChat(const LLMRequest& request) const;

    /**
     * Validation
     */
    void validateRequest(const LLMRequest& request) const;
    void validateResponsesRequest(const OpenAI::ResponsesRequest& request) const;
    void validateChatCompletionRequest(const OpenAI::ChatCompletionRequest& request) const;
};
