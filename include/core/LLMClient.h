#pragma once
#include <juce_core/juce_core.h>
#include "LLMTypes.h"

/**
 * Abstract base class for any LLM client (OpenAI, Anthropic, local models, etc)
 */
class LLMClient {
public:
    virtual ~LLMClient() = default;
    
    /**
     * Sends a request to the LLM and invokes the callback with the response
     */
    virtual void sendRequest(const LLMRequest& request, LLMResponseCallback callback) = 0;
    
    /**
     * Sends a streaming request to the LLM (if supported)
     */
    virtual void sendStreamingRequest(const LLMRequest& request, 
                                    LLMResponseCallback onDone,
                                    LLMStreamCallback onChunk) {
        // Default implementation: just call regular sendRequest
        sendRequest(request, std::move(onDone));
    }
    
    /**
     * Get available models for this client
     */
    [[nodiscard]] virtual juce::StringArray getAvailableModels() const { return {}; }
    
    /**
     * Check if the client supports streaming
     */
    [[nodiscard]] virtual bool supportsStreaming() const { return false; }
    
    /**
     * Get the client name/type
     */
    [[nodiscard]] virtual juce::String getClientName() const = 0;
}; 