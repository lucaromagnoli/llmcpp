#include <openai/OpenAITypes.h>

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST_CASE("Parameter filtering for reasoning models", "[openai][parameter-filtering]") {
    SECTION("O3-Mini should not include temperature, top_p, top_logprobs, or truncation") {
        OpenAI::ResponsesRequest req;
        req.model = "o3-mini";
        req.input = OpenAI::ResponsesInput::fromText("Test input");
        req.temperature = 0.7;
        req.topP = 0.9;
        req.topLogprobs = 5;
        req.truncation = "auto";
        req.instructions = "Test instructions";
        req.maxOutputTokens = 100;

        json result = req.toJson();

        // These parameters should be filtered out for reasoning models
        REQUIRE(!result.contains("temperature"));
        REQUIRE(!result.contains("top_p"));
        REQUIRE(!result.contains("top_logprobs"));
        REQUIRE(!result.contains("truncation"));

        // These parameters should still be included
        REQUIRE(result.contains("model"));
        REQUIRE(result.contains("input"));
        REQUIRE(result.contains("instructions"));
        REQUIRE(result.contains("max_output_tokens"));

        REQUIRE(result["model"] == "o3-mini");
        REQUIRE(result["instructions"] == "Test instructions");
        REQUIRE(result["max_output_tokens"] == 100);
    }

    SECTION("GPT-4o should include all parameters") {
        OpenAI::ResponsesRequest req;
        req.model = "gpt-4o";
        req.input = OpenAI::ResponsesInput::fromText("Test input");
        req.temperature = 0.7;
        req.topP = 0.9;
        req.topLogprobs = 5;
        req.truncation = "auto";
        req.instructions = "Test instructions";

        json result = req.toJson();

        // All parameters should be included for non-reasoning models
        REQUIRE(result.contains("temperature"));
        REQUIRE(result.contains("top_p"));
        REQUIRE(result.contains("top_logprobs"));
        REQUIRE(result.contains("truncation"));
        REQUIRE(result.contains("instructions"));

        REQUIRE(result["temperature"] == 0.7);
        REQUIRE(result["top_p"] == 0.9);
        REQUIRE(result["top_logprobs"] == 5);
        REQUIRE(result["truncation"] == "auto");
    }

    SECTION("O1-Mini should not include temperature, top_p, top_logprobs, or truncation") {
        OpenAI::ResponsesRequest req;
        req.model = "o1-mini";
        req.input = OpenAI::ResponsesInput::fromText("Test input");
        req.temperature = 0.7;
        req.topP = 0.9;
        req.topLogprobs = 5;
        req.truncation = "auto";

        json result = req.toJson();

        // These parameters should be filtered out for reasoning models
        REQUIRE(!result.contains("temperature"));
        REQUIRE(!result.contains("top_p"));
        REQUIRE(!result.contains("top_logprobs"));
        REQUIRE(!result.contains("truncation"));
    }

    SECTION("O4-Mini should not include temperature, top_p, top_logprobs, or truncation") {
        OpenAI::ResponsesRequest req;
        req.model = "o4-mini";
        req.input = OpenAI::ResponsesInput::fromText("Test input");
        req.temperature = 0.7;
        req.topP = 0.9;
        req.topLogprobs = 5;
        req.truncation = "auto";

        json result = req.toJson();

        // These parameters should be filtered out for reasoning models
        REQUIRE(!result.contains("temperature"));
        REQUIRE(!result.contains("top_p"));
        REQUIRE(!result.contains("top_logprobs"));
        REQUIRE(!result.contains("truncation"));
    }

    SECTION("isParameterSupported method works correctly") {
        OpenAI::ResponsesRequest req;
        req.model = "o3-mini";

        // Reasoning models should not support these parameters
        REQUIRE(!req.isParameterSupported("temperature"));
        REQUIRE(!req.isParameterSupported("top_p"));
        REQUIRE(!req.isParameterSupported("top_logprobs"));
        REQUIRE(!req.isParameterSupported("truncation"));

        // But should support other parameters
        REQUIRE(req.isParameterSupported("instructions"));
        REQUIRE(req.isParameterSupported("max_output_tokens"));
        REQUIRE(req.isParameterSupported("tools"));
        REQUIRE(req.isParameterSupported("background"));

        // Test with a non-reasoning model
        req.model = "gpt-4o";
        REQUIRE(req.isParameterSupported("temperature"));
        REQUIRE(req.isParameterSupported("top_p"));
        REQUIRE(req.isParameterSupported("top_logprobs"));
        REQUIRE(req.isParameterSupported("truncation"));
    }
}
