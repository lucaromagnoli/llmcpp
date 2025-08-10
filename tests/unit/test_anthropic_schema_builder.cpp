#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "anthropic/AnthropicSchemaBuilder.h"

using json = nlohmann::json;
using namespace Anthropic;

TEST_CASE("SchemaBuilder basic functionality", "[anthropic][schema]") {
    SECTION("Builder pattern works") {
        auto schema = SchemaBuilder()
                          .setType("string")
                          .setDescription("A test string")
                          .setMinLength(1)
                          .setMaxLength(100)
                          .build();

        REQUIRE(schema["type"] == "string");
        REQUIRE(schema["description"] == "A test string");
        REQUIRE(schema["minLength"] == 1);
        REQUIRE(schema["maxLength"] == 100);
    }

    SECTION("Object schema with properties") {
        json properties = {{"name", SchemaBuilder::buildStringSchema("Person name")},
                           {"age", SchemaBuilder::buildIntegerSchema("Person age", 0, 150)}};

        auto schema = SchemaBuilder()
                          .setType("object")
                          .setProperties(properties)
                          .setRequired({"name", "age"})
                          .setAdditionalProperties(false)
                          .build();

        REQUIRE(schema["type"] == "object");
        REQUIRE(schema["properties"]["name"]["type"] == "string");
        REQUIRE(schema["properties"]["age"]["type"] == "integer");
        REQUIRE(schema["required"].size() == 2);
        REQUIRE(schema["additionalProperties"] == false);
    }

    SECTION("Array schema") {
        auto itemSchema = SchemaBuilder::buildStringSchema("Array item");
        auto schema = SchemaBuilder()
                          .setType("array")
                          .setItems(itemSchema)
                          .setMinItems(1)
                          .setMaxItems(10)
                          .build();

        REQUIRE(schema["type"] == "array");
        REQUIRE(schema["items"]["type"] == "string");
        REQUIRE(schema["minItems"] == 1);
        REQUIRE(schema["maxItems"] == 10);
    }
}

TEST_CASE("SchemaBuilder static convenience methods", "[anthropic][schema]") {
    SECTION("buildStringSchema") {
        auto schema = SchemaBuilder::buildStringSchema("Test description", 5, 50);

        REQUIRE(schema["type"] == "string");
        REQUIRE(schema["description"] == "Test description");
        REQUIRE(schema["minLength"] == 5);
        REQUIRE(schema["maxLength"] == 50);
    }

    SECTION("buildIntegerSchema") {
        auto schema = SchemaBuilder::buildIntegerSchema("Test integer", 10, 100);

        REQUIRE(schema["type"] == "integer");
        REQUIRE(schema["description"] == "Test integer");
        REQUIRE(schema["minimum"] == 10);
        REQUIRE(schema["maximum"] == 100);
    }

    SECTION("buildNumberSchema") {
        auto schema = SchemaBuilder::buildNumberSchema("Test number", 0.5, 99.9);

        REQUIRE(schema["type"] == "number");
        REQUIRE(schema["description"] == "Test number");
        REQUIRE(schema["minimum"] == 0.5);
        REQUIRE(schema["maximum"] == 99.9);
    }

    SECTION("buildArraySchema") {
        auto itemSchema = SchemaBuilder::buildStringSchema("Item");
        auto schema = SchemaBuilder::buildArraySchema(itemSchema, 2, 5);

        REQUIRE(schema["type"] == "array");
        REQUIRE(schema["items"]["type"] == "string");
        REQUIRE(schema["minItems"] == 2);
        REQUIRE(schema["maxItems"] == 5);
    }

    SECTION("buildObjectSchema") {
        json properties = {{"id", SchemaBuilder::buildIntegerSchema("ID")},
                           {"name", SchemaBuilder::buildStringSchema("Name")}};
        auto schema = SchemaBuilder::buildObjectSchema(properties, {"id", "name"});

        REQUIRE(schema["type"] == "object");
        REQUIRE(schema["properties"]["id"]["type"] == "integer");
        REQUIRE(schema["properties"]["name"]["type"] == "string");
        REQUIRE(schema["required"].size() == 2);
        REQUIRE(schema["additionalProperties"] == false);
    }
}

TEST_CASE("Musical sequence schema", "[anthropic][schema][musical]") {
    SECTION("buildMusicalSequenceSchema produces correct structure") {
        auto schema = SchemaBuilder::buildMusicalSequenceSchema();

        // Verify main structure
        REQUIRE(schema["type"] == "object");
        REQUIRE(schema["additionalProperties"] == false);
        REQUIRE(schema["required"].size() == 2);
        // Check that both sequence_data and description are in required array
        bool hasSequenceData = false;
        bool hasDescription = false;
        for (const auto& req : schema["required"]) {
            if (req == "sequence_data") hasSequenceData = true;
            if (req == "description") hasDescription = true;
        }
        REQUIRE(hasSequenceData);
        REQUIRE(hasDescription);

        // Verify sequence_data array
        auto sequenceData = schema["properties"]["sequence_data"];
        REQUIRE(sequenceData["type"] == "array");
        REQUIRE(sequenceData["description"] == "Array of musical notes");

        // Verify note schema
        auto noteSchema = sequenceData["items"];
        REQUIRE(noteSchema["type"] == "object");
        REQUIRE(noteSchema["additionalProperties"] == false);
        REQUIRE(noteSchema["required"].size() == 4);

        // Verify note properties
        auto noteProps = noteSchema["properties"];
        REQUIRE(noteProps["note"]["type"] == "integer");
        REQUIRE(noteProps["note"]["minimum"] == 21);
        REQUIRE(noteProps["note"]["maximum"] == 127);
        REQUIRE(noteProps["note"]["description"] == "MIDI note number");

        REQUIRE(noteProps["start"]["type"] == "number");
        REQUIRE(noteProps["start"]["minimum"] == 0);
        REQUIRE(noteProps["start"]["description"] == "Start time in beats");

        REQUIRE(noteProps["duration"]["type"] == "number");
        REQUIRE(noteProps["duration"]["minimum"] == 0.1);
        REQUIRE(noteProps["duration"]["description"] == "Note duration in beats");

        REQUIRE(noteProps["velocity"]["type"] == "integer");
        REQUIRE(noteProps["velocity"]["minimum"] == 1);
        REQUIRE(noteProps["velocity"]["maximum"] == 127);
        REQUIRE(noteProps["velocity"]["description"] == "Note velocity (volume)");

        // Verify description property
        auto descriptionProp = schema["properties"]["description"];
        REQUIRE(descriptionProp["type"] == "string");
        REQUIRE(descriptionProp["description"] == "Description of the musical sequence");
    }

    SECTION("Musical sequence schema is valid JSON Schema draft 2020-12") {
        auto schema = SchemaBuilder::buildMusicalSequenceSchema();

        // Should not have any legacy JSON Schema properties
        REQUIRE(schema.find("$schema") == schema.end());  // We don't need to specify version

        // Should use correct type constraints
        REQUIRE(schema["properties"]["sequence_data"]["items"]["properties"]["note"]["type"] ==
                "integer");
        REQUIRE(schema["properties"]["sequence_data"]["items"]["properties"]["start"]["type"] ==
                "number");
        REQUIRE(schema["properties"]["sequence_data"]["items"]["properties"]["duration"]["type"] ==
                "number");
        REQUIRE(schema["properties"]["sequence_data"]["items"]["properties"]["velocity"]["type"] ==
                "integer");
    }
}
