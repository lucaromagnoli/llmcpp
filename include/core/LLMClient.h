#pragma once
#include "LLMTypes.h"
#include <string>
#include <vector>

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
    virtual std::vector<std::string> getAvailableModels() const { return {}; }
    
    /**
     * Check if the client supports streaming
     */
    virtual bool supportsStreaming() const { return false; }
    
    /**
     * Get the client name/type
     */
    virtual std::string getClientName() const = 0;
}; 