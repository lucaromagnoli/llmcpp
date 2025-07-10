#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "core/LLMTypes.h"

TEST_CASE("LLMRequestConfig construction") {
    LLMRequestConfig config;
    
    CHECK(config.client.empty());
    CHECK(config.model.empty());
    CHECK(config.functionName == "llm_function");
    CHECK(config.jsonSchema.is_object());
    CHECK(config.randomness == 0.8f);
    CHECK(config.maxTokens == 200);
}

TEST_CASE("LLMRequest toString") {
    LLMRequestConfig config;
    config.client = "test_client";
    config.model = "test_model";
    config.maxTokens = 100;
    
    LLMRequest request;
    request.config = config;
    request.prompt = "Hello";
    request.inputValues = {"input1", "input2"};
    
    std::string result = request.toString();
    
    CHECK(result.find("test_client") != std::string::npos);
    CHECK(result.find("test_model") != std::string::npos);
    CHECK(result.find("Hello") != std::string::npos);
    CHECK(result.find("input1") != std::string::npos);
    CHECK(result.find("input2") != std::string::npos);
}

TEST_CASE("LLMResponse construction") {
    LLMResponse response;
    
    CHECK(response.result.is_object());
    CHECK(response.success == false);
    CHECK(response.errorMessage.empty());
    CHECK(response.responseId.empty());
}

TEST_CASE("LLMResponse toString") {
    LLMResponse response;
    response.success = true;
    response.result = nlohmann::json::object({{"key", "value"}});
    response.errorMessage = "no error";
    response.responseId = "test_id";
    
    std::string result = response.toString();
    
    CHECK(result.find("true") != std::string::npos);
    CHECK(result.find("key") != std::string::npos);
    CHECK(result.find("value") != std::string::npos);
    CHECK(result.find("test_id") != std::string::npos);
}

TEST_CASE("LLMUsage calculations") {
    LLMUsage usage;
    usage.inputTokens = 100;
    usage.outputTokens = 50;
    
    CHECK(usage.totalTokens() == 150);
    CHECK(usage.inputTokens == 100);
    CHECK(usage.outputTokens == 50);
}

TEST_CASE("LLMErrorCode utilities") {
    // Test string to error code conversion
    CHECK(LLMTypeUtils::stringToErrorCode("network error") == LLMErrorCode::NetworkError);
    CHECK(LLMTypeUtils::stringToErrorCode("authentication failed") == LLMErrorCode::AuthenticationError);
    CHECK(LLMTypeUtils::stringToErrorCode("rate limit exceeded") == LLMErrorCode::RateLimitError);
    CHECK(LLMTypeUtils::stringToErrorCode("invalid request") == LLMErrorCode::InvalidRequest);
    CHECK(LLMTypeUtils::stringToErrorCode("model not found") == LLMErrorCode::ModelNotFound);
    CHECK(LLMTypeUtils::stringToErrorCode("internal server error") == LLMErrorCode::InternalError);
    CHECK(LLMTypeUtils::stringToErrorCode("unknown error") == LLMErrorCode::Unknown);
    
    // Test error code to string conversion
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::None) == "None");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::NetworkError) == "Network Error");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::AuthenticationError) == "Authentication Error");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::RateLimitError) == "Rate Limit Error");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::InvalidRequest) == "Invalid Request");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::ModelNotFound) == "Model Not Found");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::InternalError) == "Internal Error");
    CHECK(LLMTypeUtils::errorCodeToString(LLMErrorCode::Unknown) == "Unknown Error");
} 