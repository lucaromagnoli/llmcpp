#include <catch2/catch_test_macros.hpp>

#include "openai/OpenAIClient.h"
#include "openai/OpenAITypes.h"

TEST_CASE("OpenAI Model enum functionality", "[openai][model][enum]") {
    SECTION("Model enum to string conversion") {
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4_1) == "gpt-4.1");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4_1_Mini) == "gpt-4.1-mini");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4_1_Nano) == "gpt-4.1-nano");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4o) == "gpt-4o");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4o_Mini) == "gpt-4o-mini");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_4_5) == "gpt-4.5-preview");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::GPT_3_5_Turbo) == "gpt-3.5-turbo");
        REQUIRE(OpenAIClient::modelToString(OpenAI::Model::Custom) == "custom");
    }

    SECTION("String to Model enum conversion") {
        REQUIRE(OpenAIClient::stringToModel("gpt-4.1") == OpenAI::Model::GPT_4_1);
        REQUIRE(OpenAIClient::stringToModel("gpt-4.1-mini") == OpenAI::Model::GPT_4_1_Mini);
        REQUIRE(OpenAIClient::stringToModel("gpt-4.1-nano") == OpenAI::Model::GPT_4_1_Nano);
        REQUIRE(OpenAIClient::stringToModel("gpt-4o") == OpenAI::Model::GPT_4o);
        REQUIRE(OpenAIClient::stringToModel("gpt-4o-mini") == OpenAI::Model::GPT_4o_Mini);
        REQUIRE(OpenAIClient::stringToModel("gpt-4.5-preview") == OpenAI::Model::GPT_4_5);
        REQUIRE(OpenAIClient::stringToModel("gpt-3.5-turbo") == OpenAI::Model::GPT_3_5_Turbo);
        REQUIRE(OpenAIClient::stringToModel("unknown-model") == OpenAI::Model::Custom);
    }

    SECTION("Model support checking") {
        OpenAIClient client("test-key");

        REQUIRE(client.isModelSupported(OpenAI::Model::GPT_4_1) == true);
        REQUIRE(client.isModelSupported(OpenAI::Model::GPT_4o_Mini) == true);
        REQUIRE(client.isModelSupported(OpenAI::Model::GPT_3_5_Turbo) == true);
        REQUIRE(client.isModelSupported(OpenAI::Model::Custom) == false);
    }

    SECTION("Available model enums") {
        auto models = OpenAIClient::getAvailableModelEnums();
        REQUIRE(models.size() >= 8);  // Should include all defined models

        // Check that all expected models are present
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4_1) != models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4_1_Mini) !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4_1_Nano) !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4o) != models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4o_Mini) !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_4_5) != models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::GPT_3_5_Turbo) !=
                models.end());
        REQUIRE(std::find(models.begin(), models.end(), OpenAI::Model::Custom) != models.end());
    }

    SECTION("Structured output support") {
        REQUIRE(OpenAI::supportsStructuredOutputs(OpenAI::Model::GPT_4_1) == true);
        REQUIRE(OpenAI::supportsStructuredOutputs(OpenAI::Model::GPT_4_1_Mini) == true);
        REQUIRE(OpenAI::supportsStructuredOutputs(OpenAI::Model::GPT_4o) == true);
        REQUIRE(OpenAI::supportsStructuredOutputs(OpenAI::Model::GPT_3_5_Turbo) == false);
        REQUIRE(OpenAI::supportsStructuredOutputs(OpenAI::Model::Custom) == false);
    }
}
