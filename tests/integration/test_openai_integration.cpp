#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "core/ClientManager.h"
#include "core/LLMTypes.h"
#include "openai/OpenAIClient.h"

// Integration tests for OpenAI API - requires real API key
// These tests are disabled by default to avoid accidental API usage and costs
// To enable: set environment variable LLMCPP_RUN_INTEGRATION_TESTS=1
// To set API key: set environment variable OPENAI_API_KEY=your_key_here

// Simple .env file loader
static void loadEnvFile(const std::string& filepath = ".env") {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return;  // File doesn't exist, that's okay
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Find the = separator
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove quotes if present
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                value = value.substr(1, value.size() - 2);
            }

            // Set environment variable
            setenv(key.c_str(), value.c_str(), 1);
        }
    }
}

static bool shouldRunIntegrationTests() {
    // Load .env file first
    loadEnvFile();

    const char* runTests = std::getenv("LLMCPP_RUN_INTEGRATION_TESTS");
    return runTests && std::string(runTests) == "1";
}

static std::string getApiKey() {
    // Load .env file first
    loadEnvFile();

    const char* apiKey = std::getenv("OPENAI_API_KEY");
    return apiKey ? std::string(apiKey) : "";
}

TEST_CASE("OpenAI Integration - Simple text completion", "[openai][integration][manual]") {
    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 to enable.");
    }

    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        SKIP("No API key found. Set OPENAI_API_KEY environment variable.");
    }

    SECTION("Basic text completion with gpt-4o-mini") {
        OpenAIClient client(apiKey);

        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";  // Cheaper model for testing
        config.maxTokens = 50;
        config.randomness = 0.1f;  // Low randomness for predictable results

        LLMRequest request(config, "What is 2+2? Answer with just the number.");

        std::cout << "Making API call to OpenAI..." << std::endl;
        auto response = client.sendRequest(request);

        REQUIRE(response.success == true);
        REQUIRE(response.errorMessage.empty());
        REQUIRE(!response.responseId.empty());
        REQUIRE(response.usage.inputTokens > 0);
        REQUIRE(response.usage.outputTokens > 0);

        // Check that we got some text output
        REQUIRE((response.result.contains("text") || response.result.contains("choices")));

        std::cout << "✅ API call successful!" << std::endl;
        std::cout << "Response ID: " << response.responseId << std::endl;
        std::cout << "Usage: " << response.usage.toString() << std::endl;

        if (response.result.contains("text")) {
            std::cout << "Output: " << response.result["text"].get<std::string>() << std::endl;
        }
    }
}

TEST_CASE("OpenAI Integration - Async request", "[openai][integration][manual]") {
    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 to enable.");
    }

    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        SKIP("No API key found. Set OPENAI_API_KEY environment variable.");
    }

    SECTION("Async completion") {
        OpenAIClient client(apiKey);

        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";
        config.maxTokens = 30;
        config.randomness = 0.1f;

        LLMRequest request(config, "Say 'Hello from async!' in French.");

        bool callbackInvoked = false;
        LLMResponse asyncResponse;

        std::cout << "Making async API call..." << std::endl;
        auto future = client.sendRequestAsync(request, [&](const LLMResponse& response) {
            asyncResponse = response;
            callbackInvoked = true;
            std::cout << "Async callback invoked!" << std::endl;
        });

        // Wait for completion
        auto result = future.get();

        REQUIRE(callbackInvoked == true);
        REQUIRE(asyncResponse.success == true);
        REQUIRE(result.success == true);
        REQUIRE(!result.responseId.empty());

        std::cout << "✅ Async API call successful!" << std::endl;
        std::cout << "Response ID: " << result.responseId << std::endl;
    }
}

TEST_CASE("OpenAI Integration - Error handling", "[openai][integration][manual]") {
    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 to enable.");
    }

    SECTION("Invalid API key") {
        OpenAIClient client("invalid-api-key-12345");

        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";
        config.maxTokens = 10;

        LLMRequest request(config, "Test prompt");

        std::cout << "Testing invalid API key..." << std::endl;
        auto response = client.sendRequest(request);

        REQUIRE(response.success == false);
        REQUIRE(!response.errorMessage.empty());

        // Check error message contains relevant keywords (separate assertions)
        bool hasApiError = (response.errorMessage.find("api") != std::string::npos);
        bool hasKeyError = (response.errorMessage.find("key") != std::string::npos);
        bool hasAuthError = (response.errorMessage.find("auth") != std::string::npos);
        REQUIRE((hasApiError || hasKeyError || hasAuthError));

        std::cout << "✅ Error handling working correctly!" << std::endl;
        std::cout << "Error: " << response.errorMessage << std::endl;
    }
}

TEST_CASE("OpenAI Integration - Responses API specific", "[openai][integration][manual]") {
    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 to enable.");
    }

    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        SKIP("No API key found. Set OPENAI_API_KEY environment variable.");
    }

    SECTION("Structured output with function") {
        OpenAIClient client(apiKey);

        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";
        config.maxTokens = 100;
        config.randomness = 0.1f;
        config.functionName = "analyze_sentiment";
        config.jsonSchema =
            "{"
            "\"type\": \"object\","
            "\"properties\": {"
            "\"sentiment\": {\"type\": \"string\", \"enum\": [\"positive\", \"negative\", "
            "\"neutral\"]},"
            "\"confidence\": {\"type\": \"number\", \"minimum\": 0, \"maximum\": 1}"
            "},"
            "\"required\": [\"sentiment\", \"confidence\"]"
            "}";

        LLMRequest request(config, "I absolutely love this new phone! It's fantastic.");

        std::cout << "Testing Responses API with structured output..." << std::endl;
        auto response = client.sendRequest(request);

        REQUIRE(response.success == true);
        REQUIRE(!response.responseId.empty());

        // Validate the structured response conforms to our JSON schema
        REQUIRE(response.result.contains("text"));

        // Parse the structured output
        auto responseText = response.result["text"].get<std::string>();
        std::cout << "Structured output: " << responseText << std::endl;

        // For sentiment analysis, the response should be JSON-formatted
        try {
            auto parsedOutput = json::parse(responseText);

            // Validate required fields exist
            REQUIRE(parsedOutput.contains("sentiment"));
            REQUIRE(parsedOutput.contains("confidence"));

            // Validate sentiment is one of the allowed enum values
            std::string sentiment = parsedOutput["sentiment"].get<std::string>();
            REQUIRE((sentiment == "positive" || sentiment == "negative" || sentiment == "neutral"));

            // Validate confidence is a number between 0 and 1
            REQUIRE(parsedOutput["confidence"].is_number());
            double confidence = parsedOutput["confidence"].get<double>();
            REQUIRE(confidence >= 0.0);
            REQUIRE(confidence <= 1.0);

            // For this positive sentiment input, we expect positive sentiment
            REQUIRE(sentiment == "positive");
            REQUIRE(confidence > 0.5);  // Should be fairly confident about positive sentiment

            std::cout << "✅ JSON Schema validation passed!" << std::endl;
            std::cout << "Sentiment: " << sentiment << ", Confidence: " << confidence << std::endl;

        } catch (const json::exception& e) {
            FAIL("Response is not valid JSON: " + std::string(e.what()));
        }

        std::cout << "✅ Responses API call successful!" << std::endl;
        std::cout << "Response ID: " << response.responseId << std::endl;
    }
}

TEST_CASE("OpenAI Integration - ClientManager", "[openai][integration][manual]") {
    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 to enable.");
    }

    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        SKIP("No API key found. Set OPENAI_API_KEY environment variable.");
    }

    SECTION("Use OpenAI through ClientManager") {
        ClientManager manager;

        // Create and register the OpenAI client with the manager
        auto openaiClient = manager.createClient<OpenAIClient>("openai", apiKey);
        REQUIRE(openaiClient != nullptr);
        REQUIRE(openaiClient->getClientName() == "OpenAI");

        // Verify we can get it back from the manager
        auto client = manager.getClient("openai");
        REQUIRE(client != nullptr);
        REQUIRE(client->getClientName() == "OpenAI");

        LLMRequestConfig requestConfig;
        requestConfig.client = "openai";
        requestConfig.model = "gpt-4o-mini";
        requestConfig.maxTokens = 20;
        requestConfig.randomness = 0.1f;

        LLMRequest request(requestConfig, "Count to 3");

        std::cout << "Testing OpenAI through ClientManager..." << std::endl;
        auto response = openaiClient->sendRequest(request);

        REQUIRE(response.success == true);
        REQUIRE(!response.responseId.empty());

        std::cout << "✅ ClientManager integration successful!" << std::endl;
        std::cout << "Response: " << response.result.dump(2) << std::endl;
    }
}

// Manual test runner helper
TEST_CASE("Integration test instructions", "[integration][info]") {
    std::cout << "\n🔧 To run OpenAI integration tests:\n" << std::endl;
    std::cout << "1. Set your OpenAI API key:" << std::endl;
    std::cout << "   export OPENAI_API_KEY='your-api-key-here'" << std::endl;
    std::cout << "\n2. Enable integration tests:" << std::endl;
    std::cout << "   export LLMCPP_RUN_INTEGRATION_TESTS=1" << std::endl;
    std::cout << "\n3. Run the tests:" << std::endl;
    std::cout << "   cd build && ./tests/llmcpp_tests '[openai][integration]'" << std::endl;
    std::cout << "\n⚠️  Note: These tests make real API calls and will incur charges!" << std::endl;
    std::cout << "💡 Tip: Use gpt-4o-mini for cheaper testing" << std::endl;

    REQUIRE(true);  // Always pass - this is just informational
}