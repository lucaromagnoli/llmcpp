#include "openai/OpenAIHttpClient.h"

#include <httplib.h>

#include <chrono>
#include <future>
#include <sstream>
#include <stdexcept>
#include <thread>

/**
 * Private implementation class using Pimpl idiom
 */
class OpenAIHttpClient::HttpClientImpl {
   public:
    explicit HttpClientImpl(const OpenAI::OpenAIConfig& config) : config_(config) {
        // Create HTTPS client for OpenAI API
        auto baseUrl = config_.baseUrl;
        if (baseUrl.find("://") != std::string::npos) {
            // Extract hostname from full URL
            auto protocolEnd = baseUrl.find("://") + 3;
            auto pathStart = baseUrl.find('/', protocolEnd);
            if (pathStart != std::string::npos) {
                hostname_ = baseUrl.substr(protocolEnd, pathStart - protocolEnd);
                basePath_ = baseUrl.substr(pathStart);
            } else {
                hostname_ = baseUrl.substr(protocolEnd);
                basePath_ = "";
            }
        } else {
            hostname_ = "api.openai.com";
            basePath_ = "/v1";
        }

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        client_ = std::make_unique<httplib::SSLClient>(hostname_);

        // Configure client
        client_->set_connection_timeout(config_.timeoutSeconds);
        client_->set_read_timeout(config_.timeoutSeconds);
        client_->set_write_timeout(config_.timeoutSeconds);

        // Enable SSL verification
        client_->enable_server_certificate_verification(true);
#else
        throw std::runtime_error(
            "SSL support not available. Please ensure OpenSSL is properly linked.");
#endif
    }

    OpenAIHttpClient::HttpResponse post(const std::string& endpoint, const json& requestBody) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto headers = buildHeaders();
        auto url = buildUrl(endpoint);
        auto bodyStr = requestBody.dump();

        auto result = client_->Post(url, headers, bodyStr, "application/json");

        return processResponse(result);
#else
        OpenAIHttpClient::HttpResponse response;
        response.success = false;
        response.statusCode = 0;
        response.errorMessage = "SSL support not available";
        return response;
#endif
    }

    OpenAIHttpClient::HttpResponse get(const std::string& endpoint) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        auto headers = buildHeaders();
        auto url = buildUrl(endpoint);

        auto result = client_->Get(url, headers);

        return processResponse(result);
#else
        OpenAIHttpClient::HttpResponse response;
        response.success = false;
        response.statusCode = 0;
        response.errorMessage = "SSL support not available";
        return response;
#endif
    }

    void setConfig(const OpenAI::OpenAIConfig& config) {
        config_ = config;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        // Update client timeouts
        client_->set_connection_timeout(config_.timeoutSeconds);
        client_->set_read_timeout(config_.timeoutSeconds);
        client_->set_write_timeout(config_.timeoutSeconds);
#endif
    }

    OpenAI::OpenAIConfig getConfig() const { return config_; }

   private:
    OpenAI::OpenAIConfig config_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    std::unique_ptr<httplib::SSLClient> client_;
#else
    std::unique_ptr<httplib::Client> client_;  // Use regular HTTP client when SSL not available
#endif
    std::string hostname_;
    std::string basePath_;

    httplib::Headers buildHeaders() const {
        httplib::Headers headers;
        headers.emplace("Authorization", "Bearer " + config_.apiKey);
        headers.emplace("User-Agent", "llmcpp/1.0.0");

        if (!config_.organization.empty()) {
            headers.emplace("OpenAI-Organization", config_.organization);
        }

        if (!config_.project.empty()) {
            headers.emplace("OpenAI-Project", config_.project);
        }

        return headers;
    }

    std::string buildUrl(const std::string& endpoint) const {
        std::string url = basePath_;
        if (!endpoint.empty()) {
            if (endpoint[0] != '/' && !basePath_.empty() && basePath_.back() != '/') {
                url += "/";
            }
            url += endpoint;
        }
        return url;
    }

    OpenAIHttpClient::HttpResponse processResponse(const httplib::Result& result) const {
        OpenAIHttpClient::HttpResponse response;

        if (!result) {
            response.success = false;
            response.statusCode = 0;
            response.errorMessage = "Network error: " + httplib::to_string(result.error());
            return response;
        }

        response.statusCode = result->status;
        response.body = result->body;

        if (response.statusCode >= 200 && response.statusCode < 300) {
            response.success = true;
        } else {
            response.success = false;
            response.errorMessage = extractErrorMessage(response.body, response.statusCode);
        }

        return response;
    }

    std::string extractErrorMessage(const std::string& body, int statusCode) const {
        try {
            auto errorJson = json::parse(body);
            auto error = safeGetOptionalJson<json>(errorJson, "error");
            if (error.has_value()) {
                auto message = safeGetOptionalJson<std::string>(error.value(), "message");
                if (message.has_value()) {
                    return message.value();
                }
            }
        } catch (const std::exception&) {
            // If JSON parsing fails, return generic message
        }

        return "HTTP " + std::to_string(statusCode) + " error";
    }
};

// OpenAIHttpClient implementation
OpenAIHttpClient::OpenAIHttpClient(const OpenAI::OpenAIConfig& config)
    : config_(config), userAgent_("llmcpp/1.0.0"), impl_(std::make_unique<HttpClientImpl>(config)) {
    validateConfig();
}

OpenAIHttpClient::~OpenAIHttpClient() = default;

OpenAIHttpClient::HttpResponse OpenAIHttpClient::post(const std::string& endpoint,
                                                      const json& requestBody) {
    validateEndpoint(endpoint);
    validateRequestBody(requestBody);

    return executeWithRetry(
        [this, &endpoint, &requestBody]() { return impl_->post(endpoint, requestBody); });
}

OpenAIHttpClient::HttpResponse OpenAIHttpClient::get(const std::string& endpoint) {
    validateEndpoint(endpoint);

    return executeWithRetry([this, &endpoint]() { return impl_->get(endpoint); });
}

std::future<OpenAIHttpClient::HttpResponse> OpenAIHttpClient::postAsync(const std::string& endpoint,
                                                                        const json& requestBody) {
    return std::async(std::launch::async,
                      [this, endpoint, requestBody]() { return post(endpoint, requestBody); });
}

std::future<OpenAIHttpClient::HttpResponse> OpenAIHttpClient::getAsync(
    const std::string& endpoint) {
    return std::async(std::launch::async, [this, endpoint]() { return get(endpoint); });
}

std::future<OpenAIHttpClient::HttpResponse> OpenAIHttpClient::postStreaming(
    const std::string& endpoint, const json& requestBody,
    std::function<void(const std::string&)> streamCallback) {
    return std::async(std::launch::async, [this, endpoint, requestBody, streamCallback]() {
        auto response = post(endpoint, requestBody);
        if (response.success && streamCallback) {
            streamCallback(response.body);
        }
        return response;
    });
}

void OpenAIHttpClient::setConfig(const OpenAI::OpenAIConfig& config) {
    config_ = config;
    impl_->setConfig(config);
    validateConfig();
}

OpenAI::OpenAIConfig OpenAIHttpClient::getConfig() const { return impl_->getConfig(); }

void OpenAIHttpClient::setTimeoutSeconds(int timeoutSeconds) {
    config_.timeoutSeconds = timeoutSeconds;
    impl_->setConfig(config_);
}

int OpenAIHttpClient::getTimeoutSeconds() const { return config_.timeoutSeconds; }

void OpenAIHttpClient::setMaxRetries(int maxRetries) { config_.maxRetries = maxRetries; }

int OpenAIHttpClient::getMaxRetries() const { return config_.maxRetries; }

void OpenAIHttpClient::setUserAgent(const std::string& userAgent) { userAgent_ = userAgent; }

std::string OpenAIHttpClient::getUserAgent() const { return userAgent_; }

void OpenAIHttpClient::addDefaultHeader(const std::string& key, const std::string& value) {
    defaultHeaders_[key] = value;
}

void OpenAIHttpClient::removeDefaultHeader(const std::string& key) { defaultHeaders_.erase(key); }

// Private helper methods
std::unordered_map<std::string, std::string> OpenAIHttpClient::buildHeaders(
    const json& requestBody [[maybe_unused]]) const {
    std::unordered_map<std::string, std::string> headers;
    headers["Authorization"] = "Bearer " + config_.apiKey;
    headers["User-Agent"] = userAgent_;

    if (!config_.organization.empty()) {
        headers["OpenAI-Organization"] = config_.organization;
    }

    if (!config_.project.empty()) {
        headers["OpenAI-Project"] = config_.project;
    }

    // Add default headers
    for (const auto& [key, value] : defaultHeaders_) {
        headers[key] = value;
    }

    return headers;
}

std::string OpenAIHttpClient::buildUrl(const std::string& endpoint) const {
    std::string url = config_.baseUrl;
    if (!endpoint.empty()) {
        if (endpoint[0] != '/' && !url.empty() && url.back() != '/') {
            url += "/";
        }
        url += endpoint;
    }
    return url;
}

OpenAIHttpClient::HttpResponse OpenAIHttpClient::handleHttpError(int statusCode,
                                                                 const std::string& body) const {
    HttpResponse response;
    response.statusCode = statusCode;
    response.body = body;
    response.success = false;

    try {
        auto errorJson = json::parse(body);
        auto error = safeGetOptionalJson<json>(errorJson, "error");
        if (error.has_value()) {
            auto message = safeGetOptionalJson<std::string>(error.value(), "message");
            if (message.has_value()) {
                response.errorMessage = message.value();
            } else {
                response.errorMessage = "HTTP " + std::to_string(statusCode) + " error";
            }
        } else {
            response.errorMessage = "HTTP " + std::to_string(statusCode) + " error";
        }
    } catch (const std::exception&) {
        response.errorMessage = "HTTP " + std::to_string(statusCode) + " error";
    }

    return response;
}

bool OpenAIHttpClient::isRetryableError(int statusCode) const {
    // Retry on network issues and some HTTP status codes
    return statusCode == 0 ||    // Network error
           statusCode == 429 ||  // Rate limit
           statusCode == 500 ||  // Internal server error
           statusCode == 502 ||  // Bad gateway
           statusCode == 503 ||  // Service unavailable
           statusCode == 504;    // Gateway timeout
}

OpenAIHttpClient::HttpResponse OpenAIHttpClient::executeWithRetry(
    std::function<HttpResponse()> requestFunc) {
    HttpResponse lastResponse;

    for (int attempt = 0; attempt <= config_.maxRetries; ++attempt) {
        lastResponse = requestFunc();

        if (lastResponse.success || !isRetryableError(lastResponse.statusCode)) {
            break;
        }

        if (attempt < config_.maxRetries) {
            waitForRetry(attempt);
        }
    }

    return lastResponse;
}

void OpenAIHttpClient::waitForRetry(int attemptNumber) const {
    // Exponential backoff: 1s, 2s, 4s, 8s...
    auto delay = std::chrono::seconds(1 << attemptNumber);
    std::this_thread::sleep_for(delay);
}

void OpenAIHttpClient::processStreamingData(const std::string& data,
                                            std::function<void(const std::string&)> callback) {
    if (callback) {
        callback(data);
    }
}

std::string OpenAIHttpClient::extractStreamingChunk(const std::string& line
                                                    [[maybe_unused]]) const {
    return line;
}

void OpenAIHttpClient::validateConfig() const {
    if (config_.apiKey.empty()) {
        throw std::invalid_argument("OpenAI API key cannot be empty");
    }

    if (config_.timeoutSeconds <= 0) {
        throw std::invalid_argument("Timeout must be positive");
    }

    if (config_.maxRetries < 0) {
        throw std::invalid_argument("Max retries cannot be negative");
    }
}

void OpenAIHttpClient::validateEndpoint(const std::string& endpoint) const {
    if (endpoint.empty()) {
        throw std::invalid_argument("Endpoint cannot be empty");
    }
}

void OpenAIHttpClient::validateRequestBody(const json& requestBody) const {
    if (requestBody.is_null()) {
        throw std::invalid_argument("Request body cannot be null");
    }
}
