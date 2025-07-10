#pragma once
#include "openai/OpenAITypes.h"
#include <memory>
#include <functional>
#include <future>
#include <string>

// Forward declarations
class OpenAIHttpClient;

/**
 * OpenAI Responses API implementation
 * 
 * This class handles the modern OpenAI Responses API which supports:
 * - Stateful conversations with response chaining
 * - Built-in tool calling (web search, file search, code interpreter, image generation)
 * - Background processing for long-running tasks
 * - Streaming with partial results
 * - Model Context Protocol (MCP) integration
 * - Multimodal inputs (text, images, files)
 */
class OpenAIResponsesApi {
public:
    explicit OpenAIResponsesApi(std::shared_ptr<OpenAIHttpClient> httpClient);
    ~OpenAIResponsesApi() = default;
    
    /**
     * Core Responses API methods
     */
    
    // Create a new response (synchronous)
    OpenAI::ResponsesResponse create(const OpenAI::ResponsesRequest& request);
    
    // Create a new response (asynchronous)
    std::future<OpenAI::ResponsesResponse> createAsync(
        const OpenAI::ResponsesRequest& request,
        std::function<void(const OpenAI::ResponsesResponse&)> callback = nullptr);
    
    // Create with streaming support
    std::future<OpenAI::ResponsesResponse> createStreaming(
        const OpenAI::ResponsesRequest& request,
        std::function<void(const std::string&)> streamCallback,
        std::function<void(const OpenAI::ResponsesResponse&)> finalCallback = nullptr);
    
    /**
     * Response management methods
     */
    
    // Retrieve an existing response by ID
    OpenAI::ResponsesResponse retrieve(const std::string& responseId);
    
    // Cancel an in-progress background response
    OpenAI::ResponsesResponse cancel(const std::string& responseId);
    
    // Delete a stored response
    bool deleteResponse(const std::string& responseId);
    
    // List input items for a response
    json listInputItems(const std::string& responseId, 
                       const std::string& after = "",
                       int limit = 20);
    
    /**
     * Background task support
     */
    
    // Check if a response is still processing
    bool isProcessing(const std::string& responseId);
    
    // Wait for a background response to complete
    OpenAI::ResponsesResponse waitForCompletion(const std::string& responseId,
                                              int timeoutSeconds = 300,
                                              int pollIntervalSeconds = 2);
    
    /**
     * Streaming helpers
     */
    
    // Resume streaming from a specific sequence number
    std::future<OpenAI::ResponsesResponse> resumeStreaming(
        const std::string& responseId,
        int startingAfter = -1,
        std::function<void(const std::string&)> streamCallback = nullptr);
    
    /**
     * Conversation management
     */
    
    // Create a follow-up response in a conversation
    OpenAI::ResponsesResponse continueConversation(
        const std::string& previousResponseId,
        const OpenAI::ResponsesInput& newInput,
        const std::optional<std::vector<OpenAI::ToolVariant>>& tools = std::nullopt);
    
    // Fork a conversation from a specific response
    OpenAI::ResponsesResponse forkConversation(
        const std::string& forkFromResponseId,
        const OpenAI::ResponsesInput& newInput,
        const std::optional<std::vector<OpenAI::ToolVariant>>& tools = std::nullopt);
    
    /**
     * Tool and approval management
     */
    
    // Handle MCP approval requests
    OpenAI::ResponsesResponse approveMcpRequest(
        const std::string& responseId,
        const std::string& approvalRequestId,
        bool approve = true);
    
    // Submit function call outputs
    OpenAI::ResponsesResponse submitFunctionOutputs(
        const std::string& responseId,
        const std::vector<OpenAI::FunctionCallOutput>& outputs);
    
    /**
     * Configuration and validation
     */
    
    // Validate a request before sending
    bool validateRequest(const OpenAI::ResponsesRequest& request, 
                        std::string& errorMessage) const;
    
    // Get supported models for the Responses API
    std::vector<std::string> getSupportedModels() const;
    
    // Check if a model supports specific features
    bool supportsBackgroundProcessing(const std::string& model) const;
    bool supportsStreaming(const std::string& model) const;
    bool supportsTools(const std::string& model) const;
    bool supportsImageGeneration(const std::string& model) const;
    bool supportsCodeInterpreter(const std::string& model) const;
    bool supportsWebSearch(const std::string& model) const;
    bool supportsMcp(const std::string& model) const;
    
private:
    std::shared_ptr<OpenAIHttpClient> httpClient_;
    
    /**
     * Internal helper methods
     */
    
    // Build the request URL for different endpoints
    std::string buildCreateUrl() const;
    std::string buildRetrieveUrl(const std::string& responseId) const;
    std::string buildCancelUrl(const std::string& responseId) const;
    std::string buildDeleteUrl(const std::string& responseId) const;
    std::string buildInputItemsUrl(const std::string& responseId) const;
    
    // Process streaming events
    void processStreamEvent(const std::string& event, 
                          std::function<void(const std::string&)> streamCallback);
    
    // Handle different response types
    OpenAI::ResponsesResponse processResponse(const json& responseJson);
    
    // Error handling
    void handleApiError(const json& errorResponse) const;
    
    // Request preprocessing
    json preprocessRequest(const OpenAI::ResponsesRequest& request) const;
    void addDefaultParameters(json& requestJson) const;
    void validateAndProcessTools(json& requestJson) const;
    
    // Response postprocessing
    void postprocessResponse(OpenAI::ResponsesResponse& response) const;
    void extractConvenienceFields(OpenAI::ResponsesResponse& response) const;
    
    // Polling helpers for background tasks
    OpenAI::ResponsesResponse pollForCompletion(const std::string& responseId,
                                              int maxAttempts,
                                              int intervalSeconds);
}; 