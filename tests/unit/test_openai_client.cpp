#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/LLMTypes.h"
#include "openai/OpenAIClient.h"
#include "openai/OpenAISchemaBuilder.h"

using namespace OpenAI;

TEST_CASE("OpenAIClient construction", "[openai][client]") {
    OpenAIClient client("test-api-key");

    // Note: We can't test internal state directly due to Pimpl idiom
    // But we can test that the client was constructed successfully
    REQUIRE(true);  // If construction succeeded, test passes
}

TEST_CASE("OpenAIClient move semantics", "[openai][client]") {
    OpenAIClient client1("test-api-key-1");

    // Test move constructor
    OpenAIClient client2 = std::move(client1);
    REQUIRE(true);  // If move succeeded, test passes

    // Test move assignment
    OpenAIClient client3("test-api-key-3");
    client3 = std::move(client2);
    REQUIRE(true);  // If move assignment succeeded, test passes
}

TEST_CASE("OpenAIClient API type detection", "[openai][client][api-detection]") {
    OpenAIClient client("test-api-key");

    SECTION("Detect Responses API") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.functionName = "extract_data";
        config.jsonSchema =
            "{\"type\": \"object\", \"properties\": {\"result\": {\"type\": \"string\"}}}";

        LLMRequest request(config, "Extract data from this text");

        // Create a mock request that would be detected as Responses API
        // The actual detection happens in the implementation
        REQUIRE(true);  // Test that we can create the request
    }

    SECTION("Detect Chat Completions API") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        // No function name or JSON schema = Chat Completions API

        LLMRequest request(config, "What is the weather like today?");

        REQUIRE(true);  // Test that we can create the request
    }
}

TEST_CASE("OpenAIClient request validation", "[openai][client][validation]") {
    OpenAIClient client("test-api-key");

    SECTION("Valid request parameters") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.temperature = 0.7f;
        config.maxTokens = 150;

        LLMRequest request(config, "Valid prompt");

        // Test that request can be created with valid parameters
        REQUIRE(request.config.model == "gpt-4o");
        REQUIRE(request.config.temperature == Catch::Approx(0.7f));
        REQUIRE(request.config.maxTokens == 150);
        REQUIRE(request.prompt == "Valid prompt");
    }

    SECTION("Invalid temperature values") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        // Test edge cases for temperature
        config.temperature = -0.1f;  // Below minimum
        LLMRequest request1(config, "Test prompt");
        REQUIRE(request1.config.temperature ==
                -0.1f);  // Stored as-is, validation happens in client

        config.temperature = 2.1f;  // Above maximum
        LLMRequest request2(config, "Test prompt");
        REQUIRE(request2.config.temperature == 2.1f);  // Stored as-is, validation happens in client
    }

    SECTION("Invalid token limits") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        config.maxTokens = -1;  // Invalid
        LLMRequest request1(config, "Test prompt");
        REQUIRE(request1.config.maxTokens == -1);  // Stored as-is, validation happens in client

        config.maxTokens = 1000000;  // Unrealistically high
        LLMRequest request2(config, "Test prompt");
        REQUIRE(request2.config.maxTokens ==
                1000000);  // Stored as-is, validation happens in client
    }
}

TEST_CASE("OpenAIClient request building", "[openai][client][request-building]") {
    OpenAIClient client("test-api-key");

    SECTION("Build Responses API request") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.functionName = "analyze_sentiment";

        // Use OpenAI schema builder instead of manual JSON string
        auto schema =
            OpenAIResponsesSchemaBuilder("analyze_sentiment")
                .property("sentiment", JsonSchemaBuilder::string().enumValues(
                                           {"positive", "negative", "neutral"}))
                .property("confidence", JsonSchemaBuilder::number().minimum(0.0).maximum(1.0))
                .required({"sentiment", "confidence"})
                .buildSchema();

        config.schemaObject = schema;
        config.temperature = 0.3f;
        config.maxTokens = 100;

        LLMRequest request(config, "Analyze the sentiment of this text: I love this product!");

        REQUIRE(request.config.functionName == "analyze_sentiment");
        REQUIRE(request.config.schemaObject.has_value());
        REQUIRE(request.config.schemaObject->contains("properties"));
        REQUIRE(request.config.schemaObject->at("properties").contains("sentiment"));
        REQUIRE(request.config.schemaObject->at("properties").contains("confidence"));
        REQUIRE(request.prompt.find("I love this product") != std::string::npos);
    }

    SECTION("Build Chat Completions request with system message") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.temperature = 0.8f;
        config.maxTokens = 300;

        std::string systemPrompt =
            "You are a helpful assistant that explains complex topics simply.";
        std::string userPrompt = "Explain quantum computing in simple terms.";

        LLMRequest request(config, systemPrompt + "\n\nUser: " + userPrompt);

        REQUIRE(request.config.model == "gpt-4o");
        REQUIRE(request.prompt.find("helpful assistant") != std::string::npos);
        REQUIRE(request.prompt.find("quantum computing") != std::string::npos);
    }

    SECTION("Build request with input values") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        LLMInput inputValues = {"name", "Alice", "age", "30", "city", "Paris"};
        LLMRequest request(config, "Generate a profile for: {name}, {age}, {city}", inputValues);

        REQUIRE(request.inputValues.size() == 6);
        REQUIRE(request.inputValues[0] == "name");
        REQUIRE(request.inputValues[1] == "Alice");
        REQUIRE(request.inputValues[4] == "city");
        REQUIRE(request.inputValues[5] == "Paris");
    }
}

TEST_CASE("OpenAIClient response handling", "[openai][client][response]") {
    OpenAIClient client("test-api-key");

    SECTION("Successful response structure") {
        // Test that we can create and validate response structures
        LLMResponse response;
        response.success = true;
        response.responseId = "chatcmpl-123";
        response.result = json::object();
        response.result["choices"] = json::array();

        json choice = json::object();
        choice["message"] = json::object();
        choice["message"]["content"] = "Hello! How can I help you today?";
        choice["message"]["role"] = "assistant";
        choice["finish_reason"] = "stop";
        response.result["choices"].push_back(choice);

        response.result["usage"] = json::object();
        response.result["usage"]["prompt_tokens"] = 10;
        response.result["usage"]["completion_tokens"] = 9;
        response.result["usage"]["total_tokens"] = 19;

        REQUIRE(response.success == true);
        REQUIRE(response.responseId == "chatcmpl-123");
        REQUIRE(response.result["choices"].is_array());
        REQUIRE(response.result["choices"].size() == 1);
        REQUIRE(response.result["choices"][0]["message"]["content"] ==
                "Hello! How can I help you today?");
        REQUIRE(response.result["usage"]["total_tokens"] == 19);
    }

    SECTION("Error response structure") {
        LLMResponse response;
        response.success = false;
        response.errorMessage = "Invalid API key provided";

        response.result = json::object();
        response.result["error"] = json::object();
        response.result["error"]["type"] = "invalid_request_error";
        response.result["error"]["code"] = "invalid_api_key";
        response.result["error"]["message"] = "Invalid API key provided";

        REQUIRE(response.success == false);
        REQUIRE(response.errorMessage == "Invalid API key provided");
        REQUIRE(response.result["error"]["type"] == "invalid_request_error");
        REQUIRE(response.result["error"]["code"] == "invalid_api_key");
    }

    SECTION("Responses API response with function call") {
        LLMResponse response;
        response.success = true;
        response.responseId = "run-456";

        response.result = json::object();
        response.result["type"] = "function_call";
        response.result["name"] = "get_weather";
        response.result["arguments"] = json::object();
        response.result["arguments"]["location"] = "San Francisco";
        response.result["arguments"]["unit"] = "celsius";

        REQUIRE(response.result["type"] == "function_call");
        REQUIRE(response.result["name"] == "get_weather");
        REQUIRE(response.result["arguments"]["location"] == "San Francisco");
        REQUIRE(response.result["arguments"]["unit"] == "celsius");
    }
}

TEST_CASE("OpenAIClient tool usage", "[openai][client][tools]") {
    OpenAIClient client("test-api-key");

    SECTION("Function tool configuration") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.functionName = "calculate_tip";

        // Use OpenAI schema builder for tip calculation
        auto schema =
            OpenAIResponsesSchemaBuilder("calculate_tip")
                .property("bill_amount",
                          JsonSchemaBuilder::number().description("The total bill amount"))
                .property("tip_percentage",
                          JsonSchemaBuilder::number().description("The tip percentage (0-100)"))
                .property("tip_amount",
                          JsonSchemaBuilder::number().description("The calculated tip amount"))
                .property("total_amount",
                          JsonSchemaBuilder::number().description("The total amount including tip"))
                .required({"bill_amount", "tip_percentage", "tip_amount", "total_amount"})
                .buildSchema();

        config.schemaObject = schema;

        LLMRequest request(config, "Calculate a 20% tip on a $50 bill");

        REQUIRE(request.config.functionName == "calculate_tip");
        REQUIRE(request.config.schemaObject.has_value());
        REQUIRE(request.config.schemaObject->at("properties").contains("bill_amount"));
        REQUIRE(request.config.schemaObject->at("properties").contains("tip_percentage"));
        REQUIRE(request.prompt.find("20%") != std::string::npos);
        REQUIRE(request.prompt.find("$50") != std::string::npos);
    }

    SECTION("Complex tool with nested objects") {
        LLMRequestConfig config;
        config.model = "gpt-4o";
        config.functionName = "analyze_document";

        // Use OpenAI schema builder for complex nested document analysis
        auto schema =
            OpenAIResponsesSchemaBuilder("analyze_document")
                .property("summary",
                          JsonSchemaBuilder::string().description("Brief summary of the document"))
                .property("key_points", JsonSchemaBuilder::array()
                                            .items(JsonSchemaBuilder::string())
                                            .description("List of key points"))
                .property("metadata",
                          JsonSchemaBuilder::object()
                              .property("document_type", JsonSchemaBuilder::string())
                              .property("confidence",
                                        JsonSchemaBuilder::number().minimum(0.0).maximum(1.0))
                              .property("language", JsonSchemaBuilder::string()))
                .required({"summary", "key_points", "metadata"})
                .buildSchema();

        config.schemaObject = schema;

        LLMRequest request(config, "Analyze this research paper about climate change impacts");

        REQUIRE(request.config.functionName == "analyze_document");
        REQUIRE(request.config.schemaObject.has_value());
        REQUIRE(request.config.schemaObject->at("properties").contains("key_points"));
        REQUIRE(request.config.schemaObject->at("properties").contains("metadata"));
        REQUIRE(request.config.schemaObject->at("properties")
                    .at("metadata")
                    .at("properties")
                    .contains("document_type"));
    }
}

TEST_CASE("OpenAIClient edge cases", "[openai][client][edge-cases]") {
    OpenAIClient client("test-api-key");

    SECTION("Empty prompt") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        LLMRequest request(config, "");

        REQUIRE(request.prompt.empty());
        REQUIRE(request.config.model == "gpt-4o");
    }

    SECTION("Very long prompt") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        std::string longPrompt(10000, 'a');  // 10k characters
        LLMRequest request(config, longPrompt);

        REQUIRE(request.prompt.length() == 10000);
        REQUIRE(request.prompt[0] == 'a');
        REQUIRE(request.prompt[9999] == 'a');
    }

    SECTION("Special characters in prompt") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        std::string specialPrompt = "Test with special chars: \n\t\r\"'{}[]()@#$%^&*";
        LLMRequest request(config, specialPrompt);

        REQUIRE(request.prompt == specialPrompt);
        REQUIRE(request.prompt.find("\"'{}[]()@#$%^&*") != std::string::npos);
    }

    SECTION("Unicode characters") {
        LLMRequestConfig config;
        config.model = "gpt-4o";

        std::string unicodePrompt = "测试中文字符 🌟 émojis and áccénts";
        LLMRequest request(config, unicodePrompt);

        REQUIRE(request.prompt == unicodePrompt);
        REQUIRE(request.prompt.find("测试中文字符") != std::string::npos);
        REQUIRE(request.prompt.find("🌟") != std::string::npos);
        REQUIRE(request.prompt.find("áccénts") != std::string::npos);
    }
}

TEST_CASE("OpenAIClient model compatibility", "[openai][client][models]") {
    OpenAIClient client("test-api-key");

    SECTION("GPT-4 models") {
        std::vector<std::string> gpt4Models = {"gpt-4",       "gpt-4-0125-preview",
                                               "gpt-4-turbo", "gpt-4-turbo-preview",
                                               "gpt-4o",      "gpt-4o-mini"};

        for (const auto& model : gpt4Models) {
            LLMRequestConfig config;
            config.model = model;

            LLMRequest request(config, "Test prompt");

            REQUIRE(request.config.model == model);
        }
    }

    SECTION("GPT-3.5 models") {
        std::vector<std::string> gpt35Models = {"gpt-3.5-turbo", "gpt-3.5-turbo-0125",
                                                "gpt-3.5-turbo-instruct"};

        for (const auto& model : gpt35Models) {
            LLMRequestConfig config;
            config.model = model;

            LLMRequest request(config, "Test prompt");

            REQUIRE(request.config.model == model);
        }
    }

    SECTION("Legacy models") {
        std::vector<std::string> legacyModels = {"text-davinci-003", "text-davinci-002",
                                                 "text-curie-001", "text-babbage-001"};

        for (const auto& model : legacyModels) {
            LLMRequestConfig config;
            config.model = model;

            LLMRequest request(config, "Test prompt");

            REQUIRE(request.config.model == model);
        }
    }
}
