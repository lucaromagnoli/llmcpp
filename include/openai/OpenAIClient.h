#pragma once

#include <juce_core/juce_core.h>
#include "core/LLMClient.h"
#include "OpenAIModels.h"
#include "OpenAIUtils.h"

class OpenAIClient : public LLMClient {
public:
    explicit OpenAIClient(const juce::String& apiKey, bool useResponsesAPI = true);

    // LLMClient interface implementation
    void sendRequest(const LLMRequest& request, LLMResponseCallback callback) override;
    void sendStreamingRequest(const LLMRequest& request, 
                            LLMResponseCallback onDone,
                            LLMStreamCallback onChunk) override;
    [[nodiscard]] juce::StringArray getAvailableModels() const override;
    [[nodiscard]] bool supportsStreaming() const override { return true; }
    [[nodiscard]] juce::String getClientName() const override { return "OpenAI"; }

    // OpenAI-specific methods
    static LLMResponse sendOpenAIRequest(const LLMRequest& request, 
                                       const juce::String& apiKey, 
                                       bool useResponsesAPI);

    static juce::String buildChatCompletionRequest(const LLMRequest& request);
    static juce::String buildResponsesRequest(const LLMRequest& request);
    static juce::String buildPayload(const LLMRequest& request, bool useResponsesAPI);
    
    static LLMResponse handleResponse(const juce::String& responseStr, bool useResponsesAPI);
    static LLMResponse handleResponsesResponse(const juce::DynamicObject& obj);
    static LLMResponse handleCompletionsResponse(const juce::DynamicObject& obj);

    [[nodiscard]] bool isUsingResponsesAPI() const { return useResponsesAPI; }
    void setUseResponsesAPI(bool useResponses);

    void listModels(const std::function<void(const std::vector<juce::String>&)>& onSuccess,
                   const std::function<void(const juce::String&)>& onError) const;

private:
    juce::String apiKey;
    bool useResponsesAPI = true;
    
    // Simple async request handling using JUCE's built-in threading
    class SimpleAsyncRequest : public juce::Thread {
    public:
        SimpleAsyncRequest(std::function<LLMResponse()> work, LLMResponseCallback callback);
        void run() override;
        
    private:
        std::function<LLMResponse()> workFunction;
        LLMResponseCallback responseCallback;
    };
}; 