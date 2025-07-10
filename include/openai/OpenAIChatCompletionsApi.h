#pragma once
#include "openai/OpenAITypes.h"
#include <memory>
#include <functional>

class OpenAIHttpClient;

/**
 * Internal API handler for OpenAI Chat Completions API
 */
class OpenAIChatCompletionsApi {
public:
    explicit OpenAIChatCompletionsApi(std::shared_ptr<OpenAIHttpClient> httpClient);
    
    /**
     * Synchronous chat completion
     */
    OpenAI::ChatCompletionResponse sendChatCompletion(const OpenAI::ChatCompletionRequest& request);
    
    /**
     * Asynchronous chat completion
     */
    std::future<OpenAI::ChatCompletionResponse> sendChatCompletionAsync(
        const OpenAI::ChatCompletionRequest& request,
        std::function<void(const OpenAI::ChatCompletionResponse&)> callback = nullptr);
    
    /**
     * Streaming chat completion
     */
    std::future<OpenAI::ChatCompletionResponse> sendChatCompletionStreaming(
        const OpenAI::ChatCompletionRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::ChatCompletionResponse&)> finalCallback = nullptr);
    
private:
    std::shared_ptr<OpenAIHttpClient> httpClient_;
    
    /**
     * Request/response processing
     */
    json buildRequestJson(const OpenAI::ChatCompletionRequest& request) const;
    OpenAI::ChatCompletionResponse parseResponse(const json& response) const;
    
    /**
     * Streaming helpers
     */
    void processStreamingResponse(const std::string& streamData,
                                 std::function<void(const std::string&)> streamCallback,
                                 std::function<void(const OpenAI::ChatCompletionResponse&)> finalCallback);
    std::string parseStreamingChunk(const std::string& chunk) const;
    bool isStreamingComplete(const std::string& chunk) const;
    
    /**
     * Validation
     */
    void validateRequest(const OpenAI::ChatCompletionRequest& request) const;
    void validateTools(const std::vector<json>& tools) const;
    void validateMessages(const std::vector<OpenAI::ChatMessage>& messages) const;
}; 