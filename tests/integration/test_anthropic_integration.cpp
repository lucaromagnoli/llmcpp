#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdlib>
#include <future>
#include <thread>

#include "anthropic/AnthropicClient.h"
#include "anthropic/AnthropicTypes.h"
#include "core/ClientFactory.h"

TEST_CASE("Anthropic integration tests", "[anthropic][integration]") {
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    if (!apiKey) {
        SKIP("ANTHROPIC_API_KEY environment variable not set");
    }

    SECTION("Basic text completion with claude-3-5-haiku") {
        Anthropic::AnthropicClient client(apiKey);

        // Create a simple messages request
        Anthropic::MessagesRequest request;
        request.model = "claude-3-5-haiku-20241022";
        request.maxTokens = 50;

        // Add a user message
        Anthropic::Message userMsg;
        userMsg.role = Anthropic::MessageRole::USER;
        userMsg.content.push_back({.type = "text", .text = "Say hello!"});
        request.messages.push_back(userMsg);

        auto response = client.sendMessagesRequest(request);

        REQUIRE(!response.content.empty());
        REQUIRE(response.role == "assistant");
        REQUIRE(!response.model.empty());
        REQUIRE(response.usage.inputTokens > 0);
        REQUIRE(response.usage.outputTokens > 0);
        REQUIRE(!response.stopReason.empty());
    }

    SECTION("LLMRequest interface") {
        Anthropic::AnthropicClient client(apiKey);

        // Test the common LLMRequest interface
        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 50;

        LLMRequest request(config, "Say hello!");
        auto response = client.sendRequest(request);

        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));
        REQUIRE(!response.result["text"].get<std::string>().empty());
        REQUIRE(response.usage.inputTokens > 0);
        REQUIRE(response.usage.outputTokens > 0);
    }

    SECTION("Claude model enum usage") {
        Anthropic::AnthropicClient client(apiKey, Anthropic::Model::CLAUDE_HAIKU_3_5);

        LLMRequestConfig config;
        config.model = Anthropic::toString(Anthropic::Model::CLAUDE_HAIKU_3_5);
        config.maxTokens = 50;

        LLMRequest request(config, "Reply with OK");
        auto response = client.sendRequest(request);

        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));
        REQUIRE(response.usage.totalTokens() > 0);
    }

    SECTION("Multiple models comparison") {
        Anthropic::AnthropicClient client(apiKey);

        // Test multiple models to ensure they all work
        std::vector<std::string> modelsToTest = {"claude-3-5-haiku-20241022",
                                                 "claude-3-5-sonnet-20241022"};

        for (const auto& model : modelsToTest) {
            LLMRequestConfig config;
            config.model = model;
            config.maxTokens = 30;

            LLMRequest request(config, "Count to 3");
            auto response = client.sendRequest(request);

            REQUIRE(response.success);
            REQUIRE(response.result.contains("text"));
            REQUIRE(response.usage.totalTokens() > 0);

            // Each model should return some content
            auto text = response.result["text"].get<std::string>();
            REQUIRE(!text.empty());
        }
    }

    SECTION("Context conversation") {
        Anthropic::AnthropicClient client(apiKey);

        // Test conversation with context
        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 100;

        // Create context with previous conversation
        LLMContext context = {
            {{"role", "user"}, {"content", "My name is Alice"}},
            {{"role", "assistant"}, {"content", "Hello Alice! Nice to meet you."}}};

        LLMRequest request(config, "What is my name?", context);
        auto response = client.sendRequest(request);

        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));

        auto text = response.result["text"].get<std::string>();
        REQUIRE(!text.empty());
        // Should contain reference to Alice (case insensitive)
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        REQUIRE(text.find("alice") != std::string::npos);
    }

    SECTION("Temperature and parameters") {
        Anthropic::AnthropicClient client(apiKey);

        // Test with specific temperature
        Anthropic::MessagesRequest request;
        request.model = "claude-3-5-haiku-20241022";
        request.maxTokens = 50;
        request.temperature = 0.1;  // Low temperature for more deterministic output

        Anthropic::Message userMsg;
        userMsg.role = Anthropic::MessageRole::USER;
        userMsg.content.push_back({.type = "text", .text = "What is 2+2?"});
        request.messages.push_back(userMsg);

        auto response = client.sendMessagesRequest(request);

        REQUIRE(!response.content.empty());
        REQUIRE(response.usage.inputTokens > 0);
        REQUIRE(response.usage.outputTokens > 0);

        // Should contain "4" in the response
        std::string fullText;
        for (const auto& content : response.content) {
            if (content.type == "text") {
                fullText += content.text;
            }
        }
        REQUIRE(fullText.find("4") != std::string::npos);
    }

    SECTION("Async callback interface") {
        Anthropic::AnthropicClient client(apiKey);

        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 50;

        LLMRequest request(config, "Say goodbye!");

        // Test async callback
        std::promise<LLMResponse> responsePromise;
        auto future = responsePromise.get_future();

        client.sendRequest(request, [&responsePromise](const LLMResponse& response) {
            responsePromise.set_value(response);
        });

        // Wait for response with timeout
        auto status = future.wait_for(std::chrono::seconds(30));
        REQUIRE(status == std::future_status::ready);

        auto response = future.get();
        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));
        REQUIRE(!response.result["text"].get<std::string>().empty());
    }

    SECTION("Client configuration and defaults") {
        // Test different client configurations
        Anthropic::AnthropicConfig config(apiKey);
        config.defaultModel = Anthropic::Model::CLAUDE_HAIKU_3_5;
        config.timeoutSeconds = 60;

        Anthropic::AnthropicClient client(config);

        REQUIRE(client.getClientName() == "AnthropicClient");
        REQUIRE(!client.supportsStreaming());  // Not implemented yet

        auto models = client.getAvailableModels();
        REQUIRE(!models.empty());
        REQUIRE(std::find(models.begin(), models.end(), "claude-3-5-haiku-20241022") !=
                models.end());
    }

    SECTION("Error handling - invalid model") {
        Anthropic::AnthropicClient client(apiKey);

        LLMRequestConfig config;
        config.model = "invalid-model-name";
        config.maxTokens = 50;

        LLMRequest request(config, "Test");
        auto response = client.sendRequest(request);

        REQUIRE_FALSE(response.success);
        REQUIRE(!response.errorMessage.empty());
    }

    SECTION("Error handling - empty prompt") {
        Anthropic::AnthropicClient client(apiKey);

        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 50;

        LLMRequest request(config, "");  // Empty prompt
        auto response = client.sendRequest(request);

        // Should either succeed with a response or fail gracefully
        if (!response.success) {
            REQUIRE(!response.errorMessage.empty());
        }
    }

    SECTION("Large context handling") {
        Anthropic::AnthropicClient client(apiKey);

        // Test with longer context
        std::string longPrompt = "Write a summary of the following text: ";
        for (int i = 0; i < 100; ++i) {
            longPrompt += "This is sentence number " + std::to_string(i + 1) + ". ";
        }

        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 200;

        LLMRequest request(config, longPrompt);
        auto response = client.sendRequest(request);

        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));
        REQUIRE(response.usage.inputTokens > 100);  // Should have significant input tokens
    }
}

TEST_CASE("Anthropic ClientFactory integration", "[anthropic][factory][integration]") {
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    if (!apiKey) {
        SKIP("ANTHROPIC_API_KEY environment variable not set");
    }

    SECTION("Create Anthropic client via factory") {
        auto client = ClientFactory().createClient("anthropic", std::string(apiKey));

        REQUIRE(client != nullptr);
        REQUIRE(client->getClientName() == "AnthropicClient");

        // Test basic functionality
        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 50;

        LLMRequest request(config, "Test factory creation");

        std::promise<LLMResponse> responsePromise;
        auto future = responsePromise.get_future();

        client->sendRequest(request, [&responsePromise](const LLMResponse& response) {
            responsePromise.set_value(response);
        });

        auto status = future.wait_for(std::chrono::seconds(30));
        REQUIRE(status == std::future_status::ready);

        auto response = future.get();
        REQUIRE(response.success);
        REQUIRE(response.result.contains("text"));
    }

    SECTION("Factory provider validation") {
        ClientFactory factory;

        REQUIRE(factory.isProviderSupported("anthropic"));
        REQUIRE(factory.isProviderSupported("openai"));
        REQUIRE_FALSE(factory.isProviderSupported("invalid-provider"));

        auto providers = factory.getSupportedProviders();
        REQUIRE(std::find(providers.begin(), providers.end(), "anthropic") != providers.end());
        REQUIRE(std::find(providers.begin(), providers.end(), "openai") != providers.end());
    }
}

TEST_CASE("Claude-4 family models integration", "[anthropic][claude4][integration]") {
    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    if (!apiKey) {
        SKIP("ANTHROPIC_API_KEY environment variable not set");
    }

    // Test each Claude-4 model
    std::vector<std::pair<std::string, Anthropic::Model>> claude4Models = {
        {"Claude Opus 4.1", Anthropic::Model::CLAUDE_OPUS_4_1},
        {"Claude Opus 4", Anthropic::Model::CLAUDE_OPUS_4},
        {"Claude Sonnet 4", Anthropic::Model::CLAUDE_SONNET_4}};

    for (const auto& [modelName, modelEnum] : claude4Models) {
        DYNAMIC_SECTION("Testing " << modelName) {
            try {
                Anthropic::AnthropicClient client(apiKey, modelEnum);

                LLMRequestConfig config;
                config.model = Anthropic::toString(modelEnum);
                config.maxTokens = 20;
                config.temperature = 0.1;

                LLMRequest request(config, "Respond with exactly: 'Claude-4 working'");

                auto response = client.sendRequest(request);

                INFO("Model: " << modelName << " (" << Anthropic::toString(modelEnum) << ")");
                INFO("Response: " << (response.result.contains("text")
                                          ? response.result["text"].get<std::string>()
                                          : "No text"));

                REQUIRE(response.success);
                REQUIRE(response.result.contains("text"));
                REQUIRE(!response.result["text"].get<std::string>().empty());
                REQUIRE(response.usage.totalTokens() > 0);

                // Check that we actually got a response from Claude-4
                std::string responseText = response.result["text"].get<std::string>();
                // Should contain some indication it's working
                REQUIRE(responseText.length() > 5);

            } catch (const std::exception& e) {
                // Some Claude-4 models may not be available yet, so we'll log but not fail
                INFO("Model " << modelName << " may not be available: " << e.what());
                // Only fail if it's not a model availability issue
                std::string error = e.what();
                if (error.find("model") == std::string::npos &&
                    error.find("not found") == std::string::npos &&
                    error.find("invalid") == std::string::npos) {
                    FAIL("Unexpected error for " << modelName << ": " << e.what());
                }
            }
        }
    }
}
