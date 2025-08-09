#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <set>

#include "anthropic/AnthropicTypes.h"

TEST_CASE("Anthropic Model enum conversions", "[anthropic][unit]") {
    SECTION("Model to string conversion") {
        // Test Claude 4 series
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_OPUS_4_1) ==
                "claude-opus-4-1-20250805");
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_OPUS_4) == "claude-opus-4-20250514");
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_SONNET_4) ==
                "claude-sonnet-4-20250514");

        // Test Claude 3.7 series
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_SONNET_3_7) ==
                "claude-3-7-sonnet-20250219");

        // Test Claude 3.5 series
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_SONNET_3_5_V2) ==
                "claude-3-5-sonnet-20241022");
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_SONNET_3_5) ==
                "claude-3-5-sonnet-20240620");
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_HAIKU_3_5) ==
                "claude-3-5-haiku-20241022");

        // Test Claude 3 series (legacy)
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_OPUS_3) == "claude-3-opus-20240229");
        REQUIRE(Anthropic::toString(Anthropic::Model::CLAUDE_HAIKU_3) == "claude-3-haiku-20240307");

        // Test custom model
        REQUIRE(Anthropic::toString(Anthropic::Model::Custom) == "custom");
    }

    SECTION("String to model conversion") {
        // Test exact model names
        REQUIRE(Anthropic::modelFromString("claude-opus-4-1-20250805") ==
                Anthropic::Model::CLAUDE_OPUS_4_1);
        REQUIRE(Anthropic::modelFromString("claude-opus-4-20250514") ==
                Anthropic::Model::CLAUDE_OPUS_4);
        REQUIRE(Anthropic::modelFromString("claude-sonnet-4-20250514") ==
                Anthropic::Model::CLAUDE_SONNET_4);
        REQUIRE(Anthropic::modelFromString("claude-3-7-sonnet-20250219") ==
                Anthropic::Model::CLAUDE_SONNET_3_7);
        REQUIRE(Anthropic::modelFromString("claude-3-5-sonnet-20241022") ==
                Anthropic::Model::CLAUDE_SONNET_3_5_V2);
        REQUIRE(Anthropic::modelFromString("claude-3-5-sonnet-20240620") ==
                Anthropic::Model::CLAUDE_SONNET_3_5);
        REQUIRE(Anthropic::modelFromString("claude-3-5-haiku-20241022") ==
                Anthropic::Model::CLAUDE_HAIKU_3_5);
        REQUIRE(Anthropic::modelFromString("claude-3-opus-20240229") ==
                Anthropic::Model::CLAUDE_OPUS_3);
        REQUIRE(Anthropic::modelFromString("claude-3-haiku-20240307") ==
                Anthropic::Model::CLAUDE_HAIKU_3);

        // Test aliases
        REQUIRE(Anthropic::modelFromString("claude-opus-4-1") == Anthropic::Model::CLAUDE_OPUS_4_1);
        REQUIRE(Anthropic::modelFromString("claude-opus-4-0") == Anthropic::Model::CLAUDE_OPUS_4);
        REQUIRE(Anthropic::modelFromString("claude-sonnet-4-0") ==
                Anthropic::Model::CLAUDE_SONNET_4);
        REQUIRE(Anthropic::modelFromString("claude-3-7-sonnet-latest") ==
                Anthropic::Model::CLAUDE_SONNET_3_7);
        REQUIRE(Anthropic::modelFromString("claude-3-5-sonnet-latest") ==
                Anthropic::Model::CLAUDE_SONNET_3_5_V2);
        REQUIRE(Anthropic::modelFromString("claude-3-5-haiku-latest") ==
                Anthropic::Model::CLAUDE_HAIKU_3_5);

        // Test unknown model
        REQUIRE(Anthropic::modelFromString("unknown-model") == Anthropic::Model::Custom);
        REQUIRE(Anthropic::modelFromString("") == Anthropic::Model::Custom);
    }

    SECTION("Round trip conversion") {
        // Test that toString and modelFromString are inverses
        std::vector<Anthropic::Model> modelsToTest = {
            Anthropic::Model::CLAUDE_OPUS_4_1,      Anthropic::Model::CLAUDE_OPUS_4,
            Anthropic::Model::CLAUDE_SONNET_4,      Anthropic::Model::CLAUDE_SONNET_3_7,
            Anthropic::Model::CLAUDE_SONNET_3_5_V2, Anthropic::Model::CLAUDE_SONNET_3_5,
            Anthropic::Model::CLAUDE_HAIKU_3_5,     Anthropic::Model::CLAUDE_OPUS_3,
            Anthropic::Model::CLAUDE_HAIKU_3};

        for (auto model : modelsToTest) {
            std::string modelString = Anthropic::toString(model);
            auto convertedBack = Anthropic::modelFromString(modelString);
            REQUIRE(convertedBack == model);
        }
    }

    SECTION("Available models list") {
        auto models = Anthropic::getAvailableModels();

        REQUIRE(!models.empty());
        REQUIRE(models.size() >= 9);  // At least the main models we've defined

        // Check that key models are present
        REQUIRE(std::find(models.begin(), models.end(), "claude-opus-4-1-20250805") !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), "claude-3-5-haiku-20241022") !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), "claude-3-5-sonnet-20241022") !=
                models.end());

        // Verify no duplicates
        std::set<std::string> uniqueModels(models.begin(), models.end());
        REQUIRE(uniqueModels.size() == models.size());
    }
}

TEST_CASE("Anthropic MessageRole enum", "[anthropic][unit]") {
    SECTION("Role to string conversion") {
        REQUIRE(Anthropic::toString(Anthropic::MessageRole::USER) == "user");
        REQUIRE(Anthropic::toString(Anthropic::MessageRole::ASSISTANT) == "assistant");
    }
}

TEST_CASE("Anthropic MessageContent structure", "[anthropic][unit]") {
    SECTION("JSON serialization") {
        Anthropic::MessageContent content;
        content.type = "text";
        content.text = "Hello world";

        auto json = content.toJson();
        REQUIRE(json["type"] == "text");
        REQUIRE(json["text"] == "Hello world");
    }

    SECTION("Default values") {
        Anthropic::MessageContent content;
        REQUIRE(content.type == "text");
        REQUIRE(content.text.empty());
    }
}

TEST_CASE("Anthropic Message structure", "[anthropic][unit]") {
    SECTION("JSON serialization") {
        Anthropic::Message message;
        message.role = Anthropic::MessageRole::USER;
        message.content.push_back({.type = "text", .text = "Hello"});
        message.content.push_back({.type = "text", .text = "World"});

        auto json = message.toJson();
        REQUIRE(json["role"] == "user");
        REQUIRE(json["content"].is_array());
        REQUIRE(json["content"].size() == 2);
        REQUIRE(json["content"][0]["text"] == "Hello");
        REQUIRE(json["content"][1]["text"] == "World");
    }
}

TEST_CASE("Anthropic MessagesRequest structure", "[anthropic][unit]") {
    SECTION("Basic JSON serialization") {
        Anthropic::MessagesRequest request;
        request.model = "claude-3-5-haiku-20241022";
        request.maxTokens = 100;

        // Add a message
        Anthropic::Message msg;
        msg.role = Anthropic::MessageRole::USER;
        msg.content.push_back({.type = "text", .text = "Test message"});
        request.messages.push_back(msg);

        auto json = request.toJson();
        REQUIRE(json["model"] == "claude-3-5-haiku-20241022");
        REQUIRE(json["max_tokens"] == 100);
        REQUIRE(json["messages"].is_array());
        REQUIRE(json["messages"].size() == 1);
    }

    SECTION("Optional parameters") {
        Anthropic::MessagesRequest request;
        request.model = "test-model";
        request.temperature = 0.7;
        request.topP = 0.9;
        request.system = "You are a helpful assistant";
        request.stopSequences = {"stop1", "stop2"};

        auto json = request.toJson();
        REQUIRE(json["temperature"] == 0.7);
        REQUIRE(json["top_p"] == 0.9);
        REQUIRE(json["system"] == "You are a helpful assistant");
        REQUIRE(json["stop_sequences"].is_array());
        REQUIRE(json["stop_sequences"].size() == 2);
    }

    SECTION("LLMRequest conversion") {
        LLMRequestConfig config;
        config.model = "claude-3-5-haiku-20241022";
        config.maxTokens = 150;
        config.temperature = 0.5f;

        LLMContext context = {{{"role", "user"}, {"content", "Previous question"}},
                              {{"role", "assistant"}, {"content", "Previous answer"}}};

        LLMRequest llmRequest(config, "Current question", context);

        auto anthropicRequest = Anthropic::MessagesRequest::fromLLMRequest(llmRequest);

        REQUIRE(anthropicRequest.model == "claude-3-5-haiku-20241022");
        REQUIRE(anthropicRequest.maxTokens == 150);
        REQUIRE(anthropicRequest.temperature == 0.5f);
        REQUIRE(anthropicRequest.messages.size() == 3);  // 2 context + 1 current

        // Check the main prompt message
        REQUIRE(anthropicRequest.messages[0].role == Anthropic::MessageRole::USER);
        REQUIRE(anthropicRequest.messages[0].content[0].text == "Current question");

        // Check context messages
        REQUIRE(anthropicRequest.messages[1].role == Anthropic::MessageRole::USER);
        REQUIRE(anthropicRequest.messages[1].content[0].text == "Previous question");
        REQUIRE(anthropicRequest.messages[2].role == Anthropic::MessageRole::ASSISTANT);
        REQUIRE(anthropicRequest.messages[2].content[0].text == "Previous answer");
    }

    SECTION("LLMRequest conversion with invalid context") {
        LLMRequestConfig config;
        config.model = "test-model";

        // Context with invalid role
        LLMContext context = {{{"role", "invalid"}, {"content", "Should be skipped"}},
                              {{"role", "user"}, {"content", "Should be included"}}};

        LLMRequest llmRequest(config, "Main prompt", context);
        auto anthropicRequest = Anthropic::MessagesRequest::fromLLMRequest(llmRequest);

        // Should have main prompt + 1 valid context message
        REQUIRE(anthropicRequest.messages.size() == 2);
        REQUIRE(anthropicRequest.messages[1].content[0].text == "Should be included");
    }
}

TEST_CASE("Anthropic Usage structure", "[anthropic][unit]") {
    SECTION("Token calculations") {
        Anthropic::Usage usage;
        usage.inputTokens = 50;
        usage.outputTokens = 25;

        REQUIRE(usage.totalTokens() == 75);
    }

    SECTION("Default values") {
        Anthropic::Usage usage;
        REQUIRE(usage.inputTokens == 0);
        REQUIRE(usage.outputTokens == 0);
        REQUIRE(usage.totalTokens() == 0);
    }
}

TEST_CASE("Anthropic MessagesResponse structure", "[anthropic][unit]") {
    SECTION("JSON parsing") {
        json responseJson = {
            {"id", "msg_123"},
            {"type", "message"},
            {"role", "assistant"},
            {"model", "claude-3-5-haiku-20241022"},
            {"stop_reason", "end_turn"},
            {"content", json::array({{{"type", "text"}, {"text", "Hello response"}}})},
            {"usage", {{"input_tokens", 10}, {"output_tokens", 20}}}};

        auto response = Anthropic::MessagesResponse::fromJson(responseJson);

        REQUIRE(response.id == "msg_123");
        REQUIRE(response.type == "message");
        REQUIRE(response.role == "assistant");
        REQUIRE(response.model == "claude-3-5-haiku-20241022");
        REQUIRE(response.stopReason == "end_turn");
        REQUIRE(response.content.size() == 1);
        REQUIRE(response.content[0].type == "text");
        REQUIRE(response.content[0].text == "Hello response");
        REQUIRE(response.usage.inputTokens == 10);
        REQUIRE(response.usage.outputTokens == 20);
    }

    SECTION("LLMResponse conversion") {
        Anthropic::MessagesResponse anthropicResponse;
        anthropicResponse.content.push_back({.type = "text", .text = "Test response"});
        anthropicResponse.usage.inputTokens = 15;
        anthropicResponse.usage.outputTokens = 30;

        auto llmResponse = anthropicResponse.toLLMResponse();

        REQUIRE(llmResponse.success == true);
        REQUIRE(llmResponse.result.contains("text"));
        REQUIRE(llmResponse.result["text"] == "Test response");
        REQUIRE(llmResponse.usage.inputTokens == 15);
        REQUIRE(llmResponse.usage.outputTokens == 30);
        REQUIRE(llmResponse.usage.totalTokens() == 45);
    }

    SECTION("LLMResponse conversion with empty content") {
        Anthropic::MessagesResponse anthropicResponse;
        // No content added

        auto llmResponse = anthropicResponse.toLLMResponse();

        REQUIRE(llmResponse.success == false);
        REQUIRE(!llmResponse.errorMessage.empty());
    }

    SECTION("LLMResponse conversion with multiple content blocks") {
        Anthropic::MessagesResponse anthropicResponse;
        anthropicResponse.content.push_back({.type = "text", .text = "First part"});
        anthropicResponse.content.push_back({.type = "text", .text = " Second part"});

        auto llmResponse = anthropicResponse.toLLMResponse();

        REQUIRE(llmResponse.success == true);
        REQUIRE(llmResponse.result["text"] == "First part Second part");
    }
}

TEST_CASE("Anthropic AnthropicConfig structure", "[anthropic][unit]") {
    SECTION("Default configuration") {
        Anthropic::AnthropicConfig config;

        REQUIRE(config.apiKey.empty());
        REQUIRE(config.baseUrl == "https://api.anthropic.com");
        REQUIRE(config.anthropicVersion == "2023-06-01");
        REQUIRE(config.defaultModel == Anthropic::Model::CLAUDE_SONNET_3_5_V2);
        REQUIRE(config.timeoutSeconds == 30);
    }

    SECTION("Constructor with API key") {
        Anthropic::AnthropicConfig config("test-api-key");

        REQUIRE(config.apiKey == "test-api-key");
        REQUIRE(config.baseUrl == "https://api.anthropic.com");
        REQUIRE(config.defaultModel == Anthropic::Model::CLAUDE_SONNET_3_5_V2);
    }

    SECTION("Custom configuration") {
        Anthropic::AnthropicConfig config("api-key");
        config.baseUrl = "https://custom.api.com";
        config.anthropicVersion = "2024-01-01";
        config.defaultModel = Anthropic::Model::CLAUDE_HAIKU_3_5;
        config.timeoutSeconds = 60;

        REQUIRE(config.baseUrl == "https://custom.api.com");
        REQUIRE(config.anthropicVersion == "2024-01-01");
        REQUIRE(config.defaultModel == Anthropic::Model::CLAUDE_HAIKU_3_5);
        REQUIRE(config.timeoutSeconds == 60);
    }
}
