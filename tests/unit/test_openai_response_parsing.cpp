#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <iostream>

#include "openai/OpenAITypes.h"

// Helper to load the sample response
json loadSampleResponse() {
    std::ifstream file("tests/openai_response_sample.json");
    if (!file.is_open()) {
        // Try from project root if not found
        file.open("openai_response_sample.json");
    }
    REQUIRE(file.is_open());

    json responseJson;
    file >> responseJson;
    return responseJson;
}

TEST_CASE("OpenAI Response Parsing - Sample Data", "[openai][parsing]") {
    auto responseJson = loadSampleResponse();

    SECTION("JSON loading works") {
        REQUIRE(!responseJson.empty());
        REQUIRE(responseJson.contains("id"));
        REQUIRE(responseJson.contains("status"));
        REQUIRE(responseJson.contains("usage"));
        REQUIRE(responseJson.contains("output"));

        std::cout << "✅ Sample JSON loaded successfully" << std::endl;
        std::cout << "📋 Status: " << responseJson["status"].get<std::string>() << std::endl;
        std::cout << "📋 Usage tokens: " << responseJson["usage"]["total_tokens"].get<int>()
                  << std::endl;
    }

    SECTION("ResponsesResponse fromJson parsing") {
        std::cout << "\n🔍 Testing ResponsesResponse::fromJson()..." << std::endl;

        OpenAI::ResponsesResponse response;

        // Test parsing step by step
        REQUIRE_NOTHROW(response = OpenAI::ResponsesResponse::fromJson(responseJson));

        // Verify basic fields
        REQUIRE(response.id == "resp_6870e944458c8198b80495c0a4354a8f063a30c54e4b7d79");
        REQUIRE(response.object == "response");
        REQUIRE(response.status == OpenAI::ResponseStatus::Completed);
        REQUIRE(response.model == "gpt-4o-mini-2024-07-18");

        std::cout << "✅ Basic fields parsed correctly" << std::endl;
        std::cout << "📋 ID: " << response.id << std::endl;
        std::cout << "📋 Status: " << (int)response.status << std::endl;
        std::cout << "📋 Model: " << response.model << std::endl;

        // Verify usage parsing
        REQUIRE(response.usage.inputTokens == 17);
        REQUIRE(response.usage.outputTokens == 5);
        REQUIRE(response.usage.totalTokens() == 22);

        std::cout << "✅ Usage parsed correctly" << std::endl;
        std::cout << "📋 Input: " << response.usage.inputTokens << std::endl;
        std::cout << "📋 Output: " << response.usage.outputTokens << std::endl;

        // Verify output parsing
        REQUIRE(!response.output.empty());
        REQUIRE(response.output.size() == 1);

        std::cout << "✅ Output array parsed" << std::endl;
        std::cout << "📋 Output items: " << response.output.size() << std::endl;

        // Test text extraction
        std::string outputText = response.getOutputText();
        std::cout << "🔍 Extracted text: '" << outputText << "'" << std::endl;
        REQUIRE(outputText == "Hello, World!");

        std::cout << "✅ Text extraction working!" << std::endl;
    }

    SECTION("toLLMResponse conversion") {
        std::cout << "\n🔍 Testing toLLMResponse()..." << std::endl;

        auto openaiResponse = OpenAI::ResponsesResponse::fromJson(responseJson);

        LLMResponse llmResponse;
        REQUIRE_NOTHROW(llmResponse = openaiResponse.toLLMResponse());

        std::cout << "📋 LLM Success: " << llmResponse.success << std::endl;
        std::cout << "📋 LLM Error: " << llmResponse.errorMessage << std::endl;
        std::cout << "📋 LLM Response ID: " << llmResponse.responseId << std::endl;
        std::cout << "📋 LLM Usage: " << llmResponse.usage.toString() << std::endl;
        std::cout << "📋 LLM Result: " << llmResponse.result.dump(2) << std::endl;

        // Verify conversion
        REQUIRE(llmResponse.success == true);
        REQUIRE(llmResponse.errorMessage.empty());
        REQUIRE(llmResponse.responseId == "resp_6870e944458c8198b80495c0a4354a8f063a30c54e4b7d79");
        REQUIRE(llmResponse.usage.inputTokens == 17);
        REQUIRE(llmResponse.usage.outputTokens == 5);
        REQUIRE(llmResponse.result.contains("text"));
        REQUIRE(llmResponse.result["text"] == "Hello, World!");

        std::cout << "✅ toLLMResponse conversion working!" << std::endl;
    }
}

TEST_CASE("OpenAI Response Parsing - Edge Cases", "[openai][parsing]") {
    SECTION("Error response") {
        json errorResponse = {
            {"id", "resp_error_123"},
            {"object", "response"},
            {"created_at", 1752230212},
            {"status", "failed"},
            {"model", "gpt-4o-mini"},
            {"error",
             {{"message", "API rate limit exceeded"},
              {"type", "rate_limit_error"},
              {"code", "rate_limit_exceeded"}}},
            {"usage", {{"input_tokens", 10}, {"output_tokens", 0}, {"total_tokens", 10}}},
            {"output", json::array()}};

        auto response = OpenAI::ResponsesResponse::fromJson(errorResponse);
        REQUIRE(response.status == OpenAI::ResponseStatus::Failed);
        REQUIRE(response.hasError() == true);

        auto llmResponse = response.toLLMResponse();
        REQUIRE(llmResponse.success == false);
        REQUIRE(!llmResponse.errorMessage.empty());

        std::cout << "✅ Error response handling working" << std::endl;
    }

    SECTION("Empty output") {
        json emptyResponse = {
            {"id", "resp_empty_123"},
            {"object", "response"},
            {"created_at", 1752230212},
            {"status", "completed"},
            {"model", "gpt-4o-mini"},
            {"error", nullptr},
            {"usage", {{"input_tokens", 5}, {"output_tokens", 0}, {"total_tokens", 5}}},
            {"output", json::array()}};

        auto response = OpenAI::ResponsesResponse::fromJson(emptyResponse);
        REQUIRE(response.status == OpenAI::ResponseStatus::Completed);
        REQUIRE(response.output.empty());

        std::string outputText = response.getOutputText();
        REQUIRE(outputText.empty());

        std::cout << "✅ Empty output handling working" << std::endl;
    }
}

TEST_CASE("JSON utility functions work correctly") {
    SECTION("safeGetJson with existing non-null value") {
        json j = {{"key", "value"}};
        std::string result = OpenAI::safeGetJson(j, "key", std::string("default"));
        REQUIRE(result == "value");
    }

    SECTION("safeGetJson with missing key uses default") {
        json j = {{"other", "value"}};
        std::string result = OpenAI::safeGetJson(j, "missing", std::string("default"));
        REQUIRE(result == "default");
    }

    SECTION("safeGetJson with null value uses default") {
        json j = {{"key", nullptr}};
        std::string result = OpenAI::safeGetJson(j, "key", std::string("default"));
        REQUIRE(result == "default");
    }

    SECTION("safeGetOptionalJson with existing non-null value") {
        json j = {{"key", "value"}};
        auto result = OpenAI::safeGetOptionalJson<std::string>(j, "key");
        REQUIRE(result.has_value());
        REQUIRE(result.value() == "value");
    }

    SECTION("safeGetOptionalJson with missing key returns nullopt") {
        json j = {{"other", "value"}};
        auto result = OpenAI::safeGetOptionalJson<std::string>(j, "missing");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("safeGetOptionalJson with null value returns nullopt") {
        json j = {{"key", nullptr}};
        auto result = OpenAI::safeGetOptionalJson<std::string>(j, "key");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("safeGetRequiredJson with existing non-null value") {
        json j = {{"key", "value"}};
        std::string result = OpenAI::safeGetRequiredJson<std::string>(j, "key");
        REQUIRE(result == "value");
    }

    SECTION("safeGetRequiredJson with missing key throws") {
        json j = {{"other", "value"}};
        REQUIRE_THROWS_AS(OpenAI::safeGetRequiredJson<std::string>(j, "missing"),
                          std::runtime_error);
    }

    SECTION("safeGetRequiredJson with null value throws") {
        json j = {{"key", nullptr}};
        REQUIRE_THROWS_AS(OpenAI::safeGetRequiredJson<std::string>(j, "key"), std::runtime_error);
    }

    SECTION("safeGetJson works with different types") {
        json j = {{"int_key", 42}, {"bool_key", true}, {"double_key", 3.14}, {"null_key", nullptr}};

        REQUIRE(OpenAI::safeGetJson(j, "int_key", 0) == 42);
        REQUIRE(OpenAI::safeGetJson(j, "bool_key", false) == true);
        REQUIRE(OpenAI::safeGetJson(j, "double_key", 0.0) == 3.14);
        REQUIRE(OpenAI::safeGetJson(j, "null_key", 999) == 999);
        REQUIRE(OpenAI::safeGetJson(j, "missing_key", 999) == 999);
    }
}
