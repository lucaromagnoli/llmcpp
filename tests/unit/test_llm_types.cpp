#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/LLMTypes.h"

TEST_CASE("LLMRequestConfig basic functionality", "[llm][types]") {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o";
    config.temperature = 0.7f;
    config.maxTokens = 150;

    REQUIRE(config.client == "openai");
    REQUIRE(config.model == "gpt-4o");
    REQUIRE(config.temperature == Catch::Approx(0.7f));
    REQUIRE(config.maxTokens == 150);

    // Test toString method
    std::string str = config.toString();
    REQUIRE(str.find("openai") != std::string::npos);
    REQUIRE(str.find("gpt-4o") != std::string::npos);
}

TEST_CASE("LLMRequest construction and validation", "[llm][types]") {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o";

    std::string prompt = "What is the capital of France?";
    LLMContext context = {json{{"role", "user"}, {"content", "location: France"}}};

    LLMRequest request(config, prompt, context, "prev-response-123");

    REQUIRE(request.config.client == "openai");
    REQUIRE(request.config.model == "gpt-4o");
    REQUIRE(request.prompt == prompt);
    REQUIRE(request.context.size() == 1);
    REQUIRE(request.context[0]["content"] == "location: France");
    REQUIRE(request.previousResponseId == "prev-response-123");

    // Test toString method
    std::string str = request.toString();
    REQUIRE(str.find("openai") != std::string::npos);
    REQUIRE(str.find("France") != std::string::npos);
}

TEST_CASE("LLMUsage token counting", "[llm][types]") {
    LLMUsage usage;
    usage.inputTokens = 50;
    usage.outputTokens = 75;

    REQUIRE(usage.inputTokens == 50);
    REQUIRE(usage.outputTokens == 75);
    REQUIRE(usage.totalTokens() == 125);

    // Test toString method
    std::string str = usage.toString();
    REQUIRE(str.find("50") != std::string::npos);
    REQUIRE(str.find("75") != std::string::npos);
    REQUIRE(str.find("125") != std::string::npos);
}

TEST_CASE("LLMResponse success and failure states", "[llm][types]") {
    SECTION("Successful response") {
        LLMResponse response;
        response.success = true;
        response.responseId = "resp-123";
        response.result = json{{"text", "Paris is the capital of France"}};
        response.usage.inputTokens = 10;
        response.usage.outputTokens = 8;

        REQUIRE(response.success == true);
        REQUIRE(response.responseId == "resp-123");
        REQUIRE(response.result["text"] == "Paris is the capital of France");
        REQUIRE(response.usage.totalTokens() == 18);
        REQUIRE(response.errorMessage.empty());

        // Test toString method
        std::string str = response.toString();
        REQUIRE(str.find("true") != std::string::npos);
        REQUIRE(str.find("Paris") != std::string::npos);
    }

    SECTION("Failed response") {
        LLMResponse response;
        response.success = false;
        response.errorMessage = "API rate limit exceeded";

        REQUIRE(response.success == false);
        REQUIRE(response.errorMessage == "API rate limit exceeded");
        REQUIRE(response.responseId.empty());

        // Test toString method
        std::string str = response.toString();
        REQUIRE(str.find("false") != std::string::npos);
        REQUIRE(str.find("rate limit") != std::string::npos);
    }
}

TEST_CASE("LLMErrorCode string conversion", "[llm][types]") {
    REQUIRE(toString(LLMErrorCode::None) == "None");
    REQUIRE(toString(LLMErrorCode::NetworkError) == "NetworkError");
    REQUIRE(toString(LLMErrorCode::AuthenticationError) == "AuthenticationError");
    REQUIRE(toString(LLMErrorCode::RateLimitError) == "RateLimitError");
    REQUIRE(toString(LLMErrorCode::InvalidRequest) == "InvalidRequest");
    REQUIRE(toString(LLMErrorCode::ModelNotFound) == "ModelNotFound");
    REQUIRE(toString(LLMErrorCode::InternalError) == "InternalError");
    REQUIRE(toString(LLMErrorCode::Unknown) == "Unknown");
}

TEST_CASE("LLMContext vector functionality", "[llm][types]") {
    LLMContext context;

    REQUIRE(context.empty());
    REQUIRE(context.size() == 0);

    context.push_back(json{{"role", "user"}, {"content", "first"}});
    context.push_back(json{{"role", "user"}, {"content", "second"}});
    context.push_back(json{{"role", "user"}, {"content", "third"}});

    REQUIRE(context.size() == 3);
    REQUIRE(context[0]["content"] == "first");
    REQUIRE(context[1]["content"] == "second");
    REQUIRE(context[2]["content"] == "third");

    // Test range-based for loop
    std::vector<std::string> collected;
    for (const auto& item : context) {
        collected.push_back(item["content"].get<std::string>());
    }

    REQUIRE(collected.size() == 3);
    REQUIRE(collected[0] == "first");
    REQUIRE(collected[1] == "second");
    REQUIRE(collected[2] == "third");
}

TEST_CASE("LLMRequest deleted default constructor", "[llm][types]") {
    // This test ensures that LLMRequest cannot be default constructed
    // (compilation test - if this compiles, the test passes)

    LLMRequestConfig config;
    config.client = "test";
    config.model = "test-model";

    // This should compile
    LLMRequest validRequest(config, "test prompt");

    // This should NOT compile (commented out to avoid compilation error)
    // LLMRequest invalidRequest;  // Should be deleted

    REQUIRE(validRequest.config.client == "test");
}

TEST_CASE("Complex LLMRequest with all fields", "[llm][types]") {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o";
    config.functionName = "analyze_data";
    config.jsonSchema = R"({"type": "object", "properties": {"result": {"type": "string"}}})";
    config.temperature = 0.8f;  // Set temperature explicitly
    config.maxTokens = 500;

    LLMContext context = {json{{"role", "user"}, {"content", "data1"}},
                          json{{"role", "user"}, {"content", "data2"}},
                          json{{"role", "user"}, {"content", "data3"}}};
    std::string prompt = "Analyze the following data and provide insights";
    std::string previousResponseId = "response-456";

    LLMRequest request(config, prompt, context, previousResponseId);

    REQUIRE(request.config.functionName == "analyze_data");
    REQUIRE(request.config.jsonSchema.find("object") != std::string::npos);
    REQUIRE(request.context.size() == 3);
    REQUIRE(request.previousResponseId == "response-456");

    // Verify all data is preserved in toString
    std::string str = request.toString();
    REQUIRE(str.find("analyze_data") != std::string::npos);
    REQUIRE(str.find("response-456") != std::string::npos);
    REQUIRE(str.find("data1") != std::string::npos);
}

TEST_CASE("LLMResponse JSON handling", "[llm][types]") {
    LLMResponse response;
    response.success = true;

    // Test with object result
    response.result = json::object();
    response.result["type"] = "completion";
    response.result["text"] = "Generated text";
    response.result["metadata"] = json::object({{"tokens", 42}, {"model", "gpt-4o"}});

    REQUIRE(response.result.is_object());
    REQUIRE(response.result["type"] == "completion");
    REQUIRE(response.result["text"] == "Generated text");
    REQUIRE(response.result["metadata"]["tokens"] == 42);

    // Test toString with complex JSON
    std::string str = response.toString();
    REQUIRE(str.find("Generated text") != std::string::npos);
    REQUIRE(str.find("metadata") != std::string::npos);
}

TEST_CASE("LLMUsage edge cases", "[llm][types]") {
    SECTION("Zero tokens") {
        LLMUsage usage;
        REQUIRE(usage.inputTokens == 0);
        REQUIRE(usage.outputTokens == 0);
        REQUIRE(usage.totalTokens() == 0);
    }

    SECTION("Large token counts") {
        LLMUsage usage;
        usage.inputTokens = 100000;
        usage.outputTokens = 50000;

        REQUIRE(usage.totalTokens() == 150000);

        std::string str = usage.toString();
        REQUIRE(str.find("100000") != std::string::npos);
        REQUIRE(str.find("150000") != std::string::npos);
    }
}

TEST_CASE("LLMResponse complex scenarios", "[llm][types]") {
    SECTION("Response with function calls") {
        LLMResponse response;
        response.success = true;
        response.responseId = "func-call-resp";

        json functionCalls = json::array();
        functionCalls.push_back({{"id", "call_1"},
                                 {"function", "get_weather"},
                                 {"arguments", {{"location", "Paris"}}}});
        functionCalls.push_back(
            {{"id", "call_2"}, {"function", "get_time"}, {"arguments", {{"timezone", "UTC"}}}});

        response.result["function_calls"] = functionCalls;
        response.result["text"] = "I'll get the weather and time for you.";

        REQUIRE(response.result["function_calls"].is_array());
        REQUIRE(response.result["function_calls"].size() == 2);
        REQUIRE(response.result["function_calls"][0]["function"] == "get_weather");
        REQUIRE(response.result["function_calls"][1]["function"] == "get_time");
    }

    SECTION("Response with usage tracking") {
        LLMResponse response;
        response.success = true;
        response.usage.inputTokens = 120;
        response.usage.outputTokens = 80;
        response.result["text"] = "Detailed analysis completed";

        std::string str = response.toString();
        REQUIRE(str.find("120") != std::string::npos);  // input tokens
        REQUIRE(str.find("80") != std::string::npos);   // output tokens
        REQUIRE(str.find("200") != std::string::npos);  // total tokens
        REQUIRE(str.find("analysis completed") != std::string::npos);
    }
}

TEST_CASE("LLMRequestConfig optional temperature", "[llm][types][temperature]") {
    SECTION("Default config should have no temperature") {
        LLMRequestConfig config;
        REQUIRE(!config.temperature.has_value());
    }

    SECTION("Setting temperature explicitly") {
        LLMRequestConfig config;
        config.temperature = 0.5f;
        REQUIRE(config.temperature.has_value());
        REQUIRE(*config.temperature == 0.5f);
    }

    SECTION("toString with temperature set") {
        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o";
        config.temperature = 0.7f;

        std::string str = config.toString();
        REQUIRE(str.find("0.7") != std::string::npos);
        REQUIRE(str.find("temperature") != std::string::npos);
    }

    SECTION("toString without temperature") {
        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o";
        // temperature not set

        std::string str = config.toString();
        REQUIRE(str.find("not set") != std::string::npos);
        REQUIRE(str.find("temperature") != std::string::npos);
    }
}
