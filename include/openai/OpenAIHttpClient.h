#pragma once
#include "openai/OpenAITypes.h"
#include <string>
#include <memory>
#include <functional>

/**
 * HTTP client wrapper for OpenAI API calls
 */
class OpenAIHttpClient {
public:
    explicit OpenAIHttpClient(const OpenAI::OpenAIConfig& config);
    
    /**
     * HTTP Response structure
     */
    struct HttpResponse {
        int statusCode;
        std::string body;
        std::string errorMessage;
        bool success;
        
        HttpResponse() : statusCode(0), success(false) {}
    };
    
    /**
     * Synchronous HTTP requests
     */
    HttpResponse post(const std::string& endpoint, const json& requestBody);
    HttpResponse get(const std::string& endpoint);
    
    /**
     * Asynchronous HTTP requests
     */
    std::future<HttpResponse> postAsync(const std::string& endpoint, const json& requestBody);
    std::future<HttpResponse> getAsync(const std::string& endpoint);
    
    /**
     * Streaming HTTP requests
     */
    std::future<HttpResponse> postStreaming(const std::string& endpoint, 
                                           const json& requestBody,
                                           std::function<void(const std::string&)> streamCallback);
    
    /**
     * Configuration
     */
    void setConfig(const OpenAI::OpenAIConfig& config);
    OpenAI::OpenAIConfig getConfig() const;
    
    /**
     * Connection management
     */
    void setTimeoutSeconds(int timeoutSeconds);
    int getTimeoutSeconds() const;
    
    void setMaxRetries(int maxRetries);
    int getMaxRetries() const;
    
    /**
     * Request customization
     */
    void setUserAgent(const std::string& userAgent);
    std::string getUserAgent() const;
    
    void addDefaultHeader(const std::string& key, const std::string& value);
    void removeDefaultHeader(const std::string& key);
    
private:
    OpenAI::OpenAIConfig config_;
    std::string userAgent_;
    std::unordered_map<std::string, std::string> defaultHeaders_;
    
    /**
     * HTTP client implementation (using cpp-httplib)
     */
    class HttpClientImpl;
    std::unique_ptr<HttpClientImpl> impl_;
    
    /**
     * Request building
     */
    std::unordered_map<std::string, std::string> buildHeaders(const json& requestBody) const;
    std::string buildUrl(const std::string& endpoint) const;
    
    /**
     * Error handling
     */
    HttpResponse handleHttpError(int statusCode, const std::string& body) const;
    bool isRetryableError(int statusCode) const;
    
    /**
     * Retry logic
     */
    HttpResponse executeWithRetry(std::function<HttpResponse()> requestFunc);
    void waitForRetry(int attemptNumber) const;
    
    /**
     * Streaming helpers
     */
    void processStreamingData(const std::string& data, 
                             std::function<void(const std::string&)> callback);
    std::string extractStreamingChunk(const std::string& line) const;
    
    /**
     * Validation
     */
    void validateConfig() const;
    void validateEndpoint(const std::string& endpoint) const;
    void validateRequestBody(const json& requestBody) const;
}; 