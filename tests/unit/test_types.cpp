#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "core/LLMTypes.h"

TEST_CASE("LLMRequestConfig construction") {
    LLMRequestConfig config;

    REQUIRE(config.client.empty());
    REQUIRE(config.model.empty());
    REQUIRE(config.functionName == "llm_function");
    REQUIRE(config.jsonSchema.empty());
    REQUIRE(config.temperature == 0.8f);
    REQUIRE(config.maxTokens == 200);
}

TEST_CASE("LLMRequest construction with required parameters") {
    LLMRequestConfig config;
    config.client = "test_client";
    config.model = "test_model";
    config.maxTokens = 100;

    LLMRequest request(config, "Hello");

    REQUIRE(request.config.client == "test_client");
    REQUIRE(request.config.model == "test_model");
    REQUIRE(request.config.maxTokens == 100);
    REQUIRE(request.prompt == "Hello");
    REQUIRE(request.inputValues.empty());
}

TEST_CASE("LLMRequest construction with input values") {
    LLMRequestConfig config;
    config.client = "test_client";
    config.model = "test_model";

    LLMInput inputValues = {"input1", "input2"};
    LLMRequest request(config, "Hello", inputValues);

    REQUIRE(request.config.client == "test_client");
    REQUIRE(request.prompt == "Hello");
    REQUIRE(request.inputValues.size() == 2);
    REQUIRE(request.inputValues[0] == "input1");
    REQUIRE(request.inputValues[1] == "input2");
}

TEST_CASE("LLMRequest toString") {
    LLMRequestConfig config;
    config.client = "test_client";
    config.model = "test_model";
    config.maxTokens = 100;

    LLMInput inputValues = {"input1", "input2"};
    LLMRequest request(config, "Hello", inputValues);

    std::string result = request.toString();

    REQUIRE(result.find("test_client") != std::string::npos);
    REQUIRE(result.find("test_model") != std::string::npos);
    REQUIRE(result.find("Hello") != std::string::npos);
    REQUIRE(result.find("input1") != std::string::npos);
    REQUIRE(result.find("input2") != std::string::npos);
}

TEST_CASE("LLMResponse construction") {
    LLMResponse response;

    REQUIRE(response.result.is_object());
    REQUIRE(response.success == false);
    REQUIRE(response.errorMessage.empty());
    REQUIRE(response.responseId.empty());
}

TEST_CASE("LLMResponse toString") {
    LLMResponse response;
    response.success = true;
    response.result = json::object({{"key", "value"}});
    response.errorMessage = "no error";
    response.responseId = "test_id";

    std::string result = response.toString();

    REQUIRE(result.find("true") != std::string::npos);
    REQUIRE(result.find("key") != std::string::npos);
    REQUIRE(result.find("value") != std::string::npos);
    REQUIRE(result.find("test_id") != std::string::npos);
}

TEST_CASE("LLMUsage calculations") {
    LLMUsage usage;
    usage.inputTokens = 100;
    usage.outputTokens = 50;

    REQUIRE(usage.totalTokens() == 150);
    REQUIRE(usage.inputTokens == 100);
    REQUIRE(usage.outputTokens == 50);
}

TEST_CASE("LLMErrorCode string conversion") {
    // Test toString function
    REQUIRE(toString(LLMErrorCode::None) == "None");
    REQUIRE(toString(LLMErrorCode::NetworkError) == "NetworkError");
    REQUIRE(toString(LLMErrorCode::AuthenticationError) == "AuthenticationError");
    REQUIRE(toString(LLMErrorCode::RateLimitError) == "RateLimitError");
    REQUIRE(toString(LLMErrorCode::InvalidRequest) == "InvalidRequest");
    REQUIRE(toString(LLMErrorCode::ModelNotFound) == "ModelNotFound");
    REQUIRE(toString(LLMErrorCode::InternalError) == "InternalError");
    REQUIRE(toString(LLMErrorCode::Unknown) == "Unknown");
}

TEST_CASE("LLMRequestConfig with structured output") {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o";
    config.functionName = "extract_data";
    config.jsonSchema = R"({"type": "object", "properties": {"result": {"type": "string"}}})";
    config.temperature = 0.3f;
    config.maxTokens = 150;

    REQUIRE(config.functionName == "extract_data");
    REQUIRE(config.jsonSchema.find("object") != std::string::npos);
    REQUIRE(config.temperature == Catch::Approx(0.3f));
    REQUIRE(config.maxTokens == 150);
}

TEST_CASE("LLMRequest with previous response") {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = "gpt-4o";

    LLMRequest request(config, "Follow up question", {}, "previous-response-id");

    REQUIRE(request.prompt == "Follow up question");
    REQUIRE(request.previousResponseId == "previous-response-id");
}

TEST_CASE("LLMResponse with usage tracking") {
    LLMResponse response;
    response.success = true;
    response.responseId = "resp-123";
    response.result = json::object({{"text", "Generated response"}});
    response.usage.inputTokens = 50;
    response.usage.outputTokens = 25;

    REQUIRE(response.success == true);
    REQUIRE(response.responseId == "resp-123");
    REQUIRE(response.result["text"] == "Generated response");
    REQUIRE(response.usage.inputTokens == 50);
    REQUIRE(response.usage.outputTokens == 25);
    REQUIRE(response.usage.totalTokens() == 75);
}
