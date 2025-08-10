#include <catch2/catch_test_macros.hpp>

#include "core/ResponseParser.h"
#include "llmcpp.h"

using json = nlohmann::json;
using namespace llmcpp;

TEST_CASE("ResponseParser returns raw responses", "[responseparser]") {
    SECTION("Successful response returns raw data") {
        json testData = {{"message", "test content"}, {"data", {{"key", "value"}}}};

        LLMResponse response;
        response.result = testData;
        response.success = true;

        auto results =
            ResponseParser::parseStructuredResponse(response, "TestProvider", "test_function");

        REQUIRE(results.size() == 1);
        REQUIRE(results[0].source == "raw_response");
        REQUIRE(results[0].data == testData);
    }

    SECTION("Failed response returns empty") {
        LLMResponse response;
        response.success = false;
        response.errorMessage = "API error";

        auto results =
            ResponseParser::parseStructuredResponse(response, "TestProvider", "test_function");

        REQUIRE(results.empty());
    }

    SECTION("Different JSON structures are preserved as-is") {
        SECTION("Object format") {
            json objectData = {{"type", "object"}, {"content", "test"}};

            LLMResponse response;
            response.result = objectData;
            response.success = true;

            auto results =
                ResponseParser::parseStructuredResponse(response, "Provider", "function");

            REQUIRE(results.size() == 1);
            REQUIRE(results[0].data == objectData);
        }

        SECTION("Array format") {
            json arrayData = {"item1", "item2", "item3"};

            LLMResponse response;
            response.result = arrayData;
            response.success = true;

            auto results =
                ResponseParser::parseStructuredResponse(response, "Provider", "function");

            REQUIRE(results.size() == 1);
            REQUIRE(results[0].data == arrayData);
        }

        SECTION("String format") {
            json stringData = "simple string response";

            LLMResponse response;
            response.result = stringData;
            response.success = true;

            auto results =
                ResponseParser::parseStructuredResponse(response, "Provider", "function");

            REQUIRE(results.size() == 1);
            REQUIRE(results[0].data == stringData);
        }
    }
}
