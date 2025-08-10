#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include "core/LLMTypes.h"
#include "core/ResponseParser.h"

using json = nlohmann::json;
using namespace llmcpp;

TEST_CASE("ResponseParser basic functionality", "[responseparser]") {
    SECTION("Parse real Anthropic XML function call response") {
        // Real response from our Anthropic API call
        std::string realResponse = R"({
  "text": "I'll help you create a sequence for a C major triad (C-E-G) using MIDI notes 60 (C4), 64 (E4), and 67 (G4) in sequence.\n\n<function_calls>\n<invoke name=\"generate_musical_sequence\">\n<parameter name=\"sequence_data\">\n[\n  {\"note\": 60, \"start\": 0.0, \"duration\": 1.0, \"velocity\": 80},\n  {\"note\": 64, \"start\": 1.0, \"duration\": 1.0, \"velocity\": 80}, \n  {\"note\": 67, \"start\": 2.0, \"duration\": 1.0, \"velocity\": 80}\n]\n</parameter>\n<parameter name=\"description\">Simple C major triad</parameter>\n</invoke>\n</function_calls>"
})";

        LLMResponse response;
        response.result = nlohmann::json::parse(realResponse);
        response.success = true;

        auto results = ResponseParser::parseStructuredResponse(response, "Anthropic",
                                                               "generate_musical_sequence");

        // Should find at least one result
        REQUIRE(results.size() >= 1);

        // Find the result that contains our expected data
        bool foundExpectedResult = false;
        for (const auto& result : results) {
            if (result.data.is_object() && result.data.contains("sequence_data")) {
                foundExpectedResult = true;

                // Verify the parsed data structure
                REQUIRE(result.data["sequence_data"].is_array());
                REQUIRE(result.data["sequence_data"].size() == 3);

                // Check first note
                auto firstNote = result.data["sequence_data"][0];
                REQUIRE(firstNote["note"] == 60);
                REQUIRE(firstNote["start"] == 0.0);
                REQUIRE(firstNote["duration"] == 1.0);
                REQUIRE(firstNote["velocity"] == 80);

                // Check description (it's also stored in the data object)
                REQUIRE(result.data["description"] == "Simple C major triad");

                // And the separate description field should match
                REQUIRE(result.description == "Simple C major triad");

                break;
            }
        }

        REQUIRE(foundExpectedResult);
    }

    SECTION("Parse basic XML function call") {
        std::string xmlText = R"(
<function_calls>
<invoke name="test_function">
<parameter name="data">{"key": "value"}</parameter>
</invoke>
</function_calls>
)";

        auto results = ResponseParser::parseAnthropicXmlResponse(xmlText);
        REQUIRE(results.size() >= 1);

        bool foundResult = false;
        for (const auto& result : results) {
            if (result.data.contains("data") && result.data["data"].is_object() &&
                result.data["data"].contains("key") && result.data["data"]["key"] == "value") {
                foundResult = true;
                break;
            }
        }
        REQUIRE(foundResult);
    }

    SECTION("Parse JSON array from text") {
        std::string jsonText = R"(
[
  {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 80},
  {"note": 64, "start": 1.0, "duration": 1.0, "velocity": 80}
]
)";

        auto results = ResponseParser::parseJsonArrayFromText(jsonText);
        REQUIRE(results.size() >= 1);

        auto result = results[0];
        REQUIRE(result.data.is_array());
        REQUIRE(result.data.size() == 2);
        REQUIRE(result.data[0]["note"] == 60);
        REQUIRE(result.data[1]["note"] == 64);
    }
}
