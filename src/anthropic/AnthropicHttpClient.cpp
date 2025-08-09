#include "anthropic/AnthropicHttpClient.h"

#include <httplib.h>

#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

namespace Anthropic {

/**
 * PIMPL implementation for AnthropicHttpClient
 */
class AnthropicHttpClient::HttpClientImpl {
   public:
    explicit HttpClientImpl(const AnthropicConfig& config) : config_(config) {
        // Parse the base URL to extract host and setup SSL if needed
        std::string host = config_.baseUrl;

        // Remove protocol prefix
        if (host.find("https://") == 0) {
            host = host.substr(8);
            useSSL_ = true;
        } else if (host.find("http://") == 0) {
            host = host.substr(7);
            useSSL_ = false;
        } else {
            useSSL_ = true;  // Default to SSL
        }

        // Remove any trailing path
        auto pathPos = host.find('/');
        if (pathPos != std::string::npos) {
            host = host.substr(0, pathPos);
        }

        host_ = host;

        // Create HTTP/HTTPS client
        if (useSSL_) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            sslClient_ = std::make_unique<httplib::SSLClient>(host_);
            sslClient_->set_read_timeout(config_.timeoutSeconds, 0);
            sslClient_->set_write_timeout(config_.timeoutSeconds, 0);
#else
            // Fallback: construct plain HTTP client to allow compilation; will error on request
            httpClient_ = std::make_unique<httplib::Client>(host_);
            httpClient_->set_read_timeout(config_.timeoutSeconds, 0);
            httpClient_->set_write_timeout(config_.timeoutSeconds, 0);
            sslUnavailable_ = true;
#endif
        } else {
            httpClient_ = std::make_unique<httplib::Client>(host_);
            httpClient_->set_read_timeout(config_.timeoutSeconds, 0);
            httpClient_->set_write_timeout(config_.timeoutSeconds, 0);
        }
    }

    MessagesResponse sendMessagesRequest(const MessagesRequest& request) {
        // Build headers
        httplib::Headers headers = buildHeaders();

        // Convert request to JSON
        json requestJson = request.toJson();
        std::string requestBody = requestJson.dump();

        // Make the API call
        httplib::Result result;
        if (useSSL_) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
            result = sslClient_->Post("/v1/messages", headers, requestBody, "application/json");
#else
            throw std::runtime_error(
                "SSL support not available (OpenSSL>=3 not found at build time)");
#endif
        } else {
            result = httpClient_->Post("/v1/messages", headers, requestBody, "application/json");
        }

        if (!result) {
            throw std::runtime_error("HTTP request failed: Connection error");
        }

        if (result->status != 200) {
            std::string errorMsg = "HTTP " + std::to_string(result->status);
            if (!result->body.empty()) {
                try {
                    json errorJson = json::parse(result->body);
                    if (errorJson.contains("error") && errorJson["error"].contains("message")) {
                        errorMsg += ": " + errorJson["error"]["message"].get<std::string>();
                    } else {
                        errorMsg += ": " + result->body;
                    }
                } catch (...) {
                    errorMsg += ": " + result->body;
                }
            }
            throw std::runtime_error(errorMsg);
        }

        // Parse response
        try {
            json responseJson = json::parse(result->body);
            return MessagesResponse::fromJson(responseJson);
        } catch (const json::exception& e) {
            throw std::runtime_error("Failed to parse response JSON: " + std::string(e.what()));
        }
    }

   private:
    httplib::Headers buildHeaders() const {
        httplib::Headers headers;
        headers.emplace("x-api-key", config_.apiKey);
        headers.emplace("anthropic-version", config_.anthropicVersion);
        headers.emplace("User-Agent", "llmcpp/1.0");
        return headers;
    }

    AnthropicConfig config_;
    std::string host_;
    bool useSSL_ = true;
    std::unique_ptr<httplib::Client> httpClient_;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    std::unique_ptr<httplib::SSLClient> sslClient_;
#endif
    bool sslUnavailable_ = false;
};

// AnthropicHttpClient implementation
AnthropicHttpClient::AnthropicHttpClient(const AnthropicConfig& config)
    : pImpl(std::make_unique<HttpClientImpl>(config)) {}

AnthropicHttpClient::~AnthropicHttpClient() = default;

MessagesResponse AnthropicHttpClient::sendMessagesRequest(const MessagesRequest& request) {
    return pImpl->sendMessagesRequest(request);
}

}  // namespace Anthropic
