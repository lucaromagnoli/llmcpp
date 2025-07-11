#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "core/JsonSchemaBuilder.h"
#include "openai/OpenAISchemaBuilder.h"

using json = nlohmann::json;

TEST_CASE("JsonSchemaBuilder basic functionality", "[schema][core]") {
    SECTION("Simple string schema") {
        auto schema = JsonSchemaBuilder::string().minLength(1).maxLength(100).build();

        REQUIRE(schema["type"] == "string");
        REQUIRE(schema["minLength"] == 1);
        REQUIRE(schema["maxLength"] == 100);
    }

    SECTION("Object schema with properties") {
        auto schema = JsonSchemaBuilder::object()
                          .property("name", JsonSchemaBuilder::string().description("User's name"))
                          .property("age", JsonSchemaBuilder::integer().minimum(0).maximum(150))
                          .property("email", JsonSchemaBuilder::string().format("email"))
                          .required({"name", "age"})
                          .additionalProperties(false)
                          .build();

        REQUIRE(schema["type"] == "object");
        REQUIRE(schema["properties"]["name"]["type"] == "string");
        REQUIRE(schema["properties"]["name"]["description"] == "User's name");
        REQUIRE(schema["properties"]["age"]["type"] == "integer");
        REQUIRE(schema["properties"]["age"]["minimum"] == 0);
        REQUIRE(schema["properties"]["age"]["maximum"] == 150);
        REQUIRE(schema["properties"]["email"]["format"] == "email");
        REQUIRE(schema["required"].size() == 2);
        REQUIRE(schema["required"][0] == "name");
        REQUIRE(schema["required"][1] == "age");
        REQUIRE(schema["additionalProperties"] == false);
    }

    SECTION("Array schema") {
        auto schema = JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string())
                          .minItems(1)
                          .maxItems(10)
                          .uniqueItems(true)
                          .build();

        REQUIRE(schema["type"] == "array");
        REQUIRE(schema["items"]["type"] == "string");
        REQUIRE(schema["minItems"] == 1);
        REQUIRE(schema["maxItems"] == 10);
        REQUIRE(schema["uniqueItems"] == true);
    }

    SECTION("Enum schema") {
        auto schema = JsonSchemaBuilder::stringEnum({"red", "green", "blue"}).build();

        REQUIRE(schema["type"] == "string");
        REQUIRE(schema["enum"].size() == 3);
        REQUIRE(schema["enum"][0] == "red");
        REQUIRE(schema["enum"][1] == "green");
        REQUIRE(schema["enum"][2] == "blue");
    }
}

TEST_CASE("JsonSchemaBuilder advanced features", "[schema][core]") {
    SECTION("Nested object schema") {
        auto addressSchema =
            JsonSchemaBuilder::object()
                .property("street", JsonSchemaBuilder::string())
                .property("city", JsonSchemaBuilder::string())
                .property("zipCode", JsonSchemaBuilder::string().pattern("^[0-9]{5}$"))
                .required({"street", "city"});

        auto personSchema =
            JsonSchemaBuilder::object()
                .property("name", JsonSchemaBuilder::string())
                .property("address", addressSchema)
                .property("hobbies", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
                .required({"name"})
                .build();

        REQUIRE(personSchema["type"] == "object");
        REQUIRE(personSchema["properties"]["address"]["type"] == "object");
        REQUIRE(personSchema["properties"]["address"]["properties"]["zipCode"]["pattern"] ==
                "^[0-9]{5}$");
        REQUIRE(personSchema["properties"]["hobbies"]["type"] == "array");
        REQUIRE(personSchema["properties"]["hobbies"]["items"]["type"] == "string");
    }

    SECTION("Composition with anyOf") {
        auto stringOrNumber = JsonSchemaBuilder()
                                  .anyOf({JsonSchemaBuilder::string(), JsonSchemaBuilder::number()})
                                  .build();

        REQUIRE(stringOrNumber.contains("anyOf"));
        REQUIRE(stringOrNumber["anyOf"].size() == 2);
        REQUIRE(stringOrNumber["anyOf"][0]["type"] == "string");
        REQUIRE(stringOrNumber["anyOf"][1]["type"] == "number");
    }
}

TEST_CASE("OpenAI Responses API schema builder", "[schema][openai][responses]") {
    SECTION("Sentiment analysis schema") {
        auto config =
            OpenAIResponsesSchemaBuilder("sentiment_analysis")
                .description("Analyze sentiment of text")
                .property("sentiment",
                          JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
                .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
                .required({"sentiment", "confidence"})
                .build();

        auto configJson = config.toJson();
        REQUIRE(configJson["format"]["type"] == "json_schema");
        REQUIRE(configJson["format"]["name"] == "sentiment_analysis");
        REQUIRE(configJson["format"]["strict"] == true);

        auto schema = configJson["format"]["schema"];
        REQUIRE(schema["type"] == "object");
        REQUIRE(schema["properties"]["sentiment"]["enum"].size() == 3);
        REQUIRE(schema["properties"]["confidence"]["minimum"] == 0);
        REQUIRE(schema["properties"]["confidence"]["maximum"] == 1);
    }

    SECTION("Custom schema with additional properties") {
        auto config = OpenAIResponsesSchemaBuilder("data_extraction")
                          .description("Extract structured data")
                          .property("title", JsonSchemaBuilder::string())
                          .property("author", JsonSchemaBuilder::string())
                          .property("tags", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
                          .required({"title"})
                          .additionalProperties(false)
                          .strict(true)
                          .build();

        auto configJson = config.toJson();
        auto schema = configJson["format"]["schema"];
        REQUIRE(schema["additionalProperties"] == false);
        REQUIRE(schema["required"].size() == 1);
        REQUIRE(schema["required"][0] == "title");
    }
}

TEST_CASE("OpenAI Chat Completions schema builder", "[schema][openai][chat]") {
    SECTION("JSON mode") {
        auto responseFormat = OpenAIChatSchemaBuilder().jsonMode().build();

        REQUIRE(responseFormat["type"] == "json_object");
    }

    SECTION("Text mode") {
        auto responseFormat = OpenAIChatSchemaBuilder().text().build();

        REQUIRE(responseFormat["type"] == "text");
    }

    SECTION("JSON schema mode") {
        auto schema =
            JsonSchemaBuilder::object()
                .property("category", JsonSchemaBuilder::stringEnum({"tech", "science", "art"}))
                .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
                .required({"category"});

        auto responseFormat =
            OpenAIChatSchemaBuilder().jsonSchema("classification", schema).build();

        REQUIRE(responseFormat["type"] == "json_schema");
        REQUIRE(responseFormat["json_schema"]["name"] == "classification");
        REQUIRE(responseFormat["json_schema"]["strict"] == true);

        auto responseSchema = responseFormat["json_schema"]["schema"];
        REQUIRE(responseSchema["type"] == "object");
        REQUIRE(responseSchema["properties"]["category"]["enum"].size() == 3);
    }
}

TEST_CASE("Schema integration examples", "[schema][integration]") {
    SECTION("Complex nested schema for content analysis") {
        auto entitySchema =
            JsonSchemaBuilder::object()
                .property("text", JsonSchemaBuilder::string())
                .property("type",
                          JsonSchemaBuilder::stringEnum({"PERSON", "ORGANIZATION", "LOCATION"}))
                .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
                .required({"text", "type"});

        auto analysisSchema =
            OpenAIResponsesSchemaBuilder("content_analysis")
                .description("Comprehensive content analysis")
                .property("summary", JsonSchemaBuilder::string().maxLength(500))
                .property("sentiment",
                          JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
                .property("entities", JsonSchemaBuilder::arrayOf(entitySchema))
                .property("topics", JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()))
                .property("readability_score", JsonSchemaBuilder::number().minimum(0).maximum(100))
                .property("metadata",
                          JsonSchemaBuilder::object()
                              .property("word_count", JsonSchemaBuilder::integer().minimum(0))
                              .property("processing_time_ms", JsonSchemaBuilder::number())
                              .additionalProperties(false))
                .required({"summary", "sentiment", "entities"})
                .additionalProperties(false)
                .build();

        auto configJson = analysisSchema.toJson();
        auto schema = configJson["format"]["schema"];

        // Verify complex structure
        REQUIRE(schema["properties"]["entities"]["items"]["properties"].contains("confidence"));
        REQUIRE(schema["properties"]["metadata"]["properties"].contains("word_count"));
        REQUIRE(schema["properties"]["metadata"]["additionalProperties"] == false);
        REQUIRE(schema["required"].size() == 3);
    }
}
