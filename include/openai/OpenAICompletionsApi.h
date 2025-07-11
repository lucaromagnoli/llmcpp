#pragma once
#include <functional>
#include <memory>

#include "openai/OpenAITypes.h"

class OpenAIHttpClient;

/**
 * Internal API handler for OpenAI Completions API (Legacy - Deprecated)
 */
class OpenAICompletionsApi {
   public:
    explicit OpenAICompletionsApi(std::shared_ptr<OpenAIHttpClient> httpClient);

    /**
     * Synchronous completion
     */
    OpenAI::CompletionResponse sendCompletion(const OpenAI::CompletionRequest& request);

    /**
     * Asynchronous completion
     */
    std::future<OpenAI::CompletionResponse> sendCompletionAsync(
        const OpenAI::CompletionRequest& request,
        std::function<void(const OpenAI::CompletionResponse&)> callback = nullptr);

    /**
     * Streaming completion
     */
    std::future<OpenAI::CompletionResponse> sendCompletionStreaming(
        const OpenAI::CompletionRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::CompletionResponse&)> finalCallback = nullptr);

    /**
     * Deprecation warnings
     */
    void enableDeprecationWarnings(bool enable);
    bool areDeprecationWarningsEnabled() const;

   private:
    std::shared_ptr<OpenAIHttpClient> httpClient_;
    bool deprecationWarningsEnabled_;

    /**
     * Request/response processing
     */
    json buildRequestJson(const OpenAI::CompletionRequest& request) const;
    OpenAI::CompletionResponse parseResponse(const json& response) const;

    /**
     * Streaming helpers
     */
    void processStreamingResponse(
        const std::string& streamData, std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::CompletionResponse&)> finalCallback);
    std::string parseStreamingChunk(const std::string& chunk) const;
    bool isStreamingComplete(const std::string& chunk) const;

    /**
     * Validation
     */
    void validateRequest(const OpenAI::CompletionRequest& request) const;
    void validateModel(const std::string& model) const;

    /**
     * Deprecation warnings
     */
    void logDeprecationWarning(const std::string& model) const;
    void logApiDeprecationWarning() const;
    std::string getRecommendedReplacement(const std::string& model) const;
};
