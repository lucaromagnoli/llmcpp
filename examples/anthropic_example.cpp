#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "llmcpp.h"

int main() {
    // Get API key from environment
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    if (!apiKey) {
        std::cerr << "Error: ANTHROPIC_API_KEY environment variable not set" << std::endl;
        return 1;
    }

    try {
        // Create Anthropic client
        llmcpp::AnthropicClient client(apiKey);

        std::cout << "=== Anthropic Claude API Example ===" << std::endl;
        std::cout << "Using client: " << client.getClientName() << std::endl;

        // Example 1: Simple text completion using LLMRequest interface
        std::cout << "\n--- Example 1: Simple completion ---" << std::endl;

        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";  // Fast and affordable model
        config.maxTokens = 100;
        config.temperature = 0.7f;

        LLMRequest request(config, "Write a haiku about artificial intelligence.");

        auto response = client.sendRequest(request);

        if (response.success) {
            std::cout << "Response: " << response.result["text"].get<std::string>() << std::endl;
            std::cout << "Usage: " << response.usage.inputTokens << " input, "
                      << response.usage.outputTokens << " output tokens" << std::endl;
        } else {
            std::cerr << "Error: " << response.errorMessage << std::endl;
        }

        // Example 2: Using Anthropic-specific API with different models
        std::cout << "\n--- Example 2: Direct Anthropic API ---" << std::endl;

        Anthropic::MessagesRequest directRequest;
        directRequest.model = Anthropic::toString(Anthropic::Model::CLAUDE_SONNET_3_5_V2);
        directRequest.maxTokens = 150;
        directRequest.temperature = 0.3;

        // Add user message
        Anthropic::Message userMsg;
        userMsg.role = Anthropic::MessageRole::USER;
        userMsg.content.push_back(
            {.type = "text", .text = "Explain the concept of machine learning in simple terms."});
        directRequest.messages.push_back(userMsg);

        auto directResponse = client.sendMessagesRequest(directRequest);

        std::cout << "Model: " << directResponse.model << std::endl;
        std::cout << "Stop reason: " << directResponse.stopReason << std::endl;

        for (const auto& content : directResponse.content) {
            if (content.type == "text") {
                std::cout << "Response: " << content.text << std::endl;
            }
        }
        std::cout << "Usage: " << directResponse.usage.inputTokens << " input, "
                  << directResponse.usage.outputTokens << " output tokens" << std::endl;

        // Example 3: Show available models
        std::cout << "\n--- Example 3: Available models ---" << std::endl;
        auto models = client.getAvailableModels();
        std::cout << "Available Anthropic models:" << std::endl;
        for (const auto& model : models) {
            std::cout << "  - " << model << std::endl;
        }

        // Example 4: Using ClientFactory
        std::cout << "\n--- Example 4: Using ClientFactory ---" << std::endl;
        auto factoryClient = llmcpp::ClientFactory::createClient("anthropic", std::string(apiKey));
        if (factoryClient) {
            std::cout << "Created client via factory: " << factoryClient->getClientName()
                      << std::endl;

            LLMRequestConfig simpleConfig;
            simpleConfig.model = "claude-3-5-haiku-20241022";
            simpleConfig.maxTokens = 50;

            LLMRequest simpleRequest(simpleConfig, "Say hello in French.");

            // Use async version with callback
            LLMResponse simpleResponse;
            bool responseReceived = false;

            factoryClient->sendRequest(simpleRequest, [&](const LLMResponse& response) {
                simpleResponse = response;
                responseReceived = true;
            });

            // Wait for response (simple busy wait for demo)
            while (!responseReceived) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (simpleResponse.success) {
                std::cout << "Factory client response: "
                          << simpleResponse.result["text"].get<std::string>() << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
