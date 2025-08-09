#pragma once
#include <memory>
#include <string>

#include "anthropic/AnthropicTypes.h"

namespace Anthropic {

/**
 * HTTP client for Anthropic API
 */
class AnthropicHttpClient {
   public:
    explicit AnthropicHttpClient(const AnthropicConfig& config);
    ~AnthropicHttpClient();

    /**
     * Send a Messages API request
     */
    MessagesResponse sendMessagesRequest(const MessagesRequest& request);

   private:
    class HttpClientImpl;
    std::unique_ptr<HttpClientImpl> pImpl;
};

}  // namespace Anthropic
