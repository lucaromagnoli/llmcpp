#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

#include "core/LLMTypes.h"
#include "core/ResponseParser.h"

using json = nlohmann::json;
using namespace llmcpp;

TEST_CASE("ResponseParser with real Anthropic data", "[responseparser][anthropic][real]") {
    SECTION("Parse real Anthropic XML function call response") {
        // This is the actual response we got from Anthropic API
        std::string realResponse = R"({
  "text": "I'll help you create a sequence for a C major triad (C-E-G) using MIDI notes 60 (C4), 64 (E4), and 67 (G4) in sequence.\n\n<function_calls>\n<invoke name=\"generate_musical_sequence\">\n<parameter name=\"sequence_data\">\n[\n  {\"note\": 60, \"start\": 0.0, \"duration\": 1.0, \"velocity\": 80},\n  {\"note\": 64, \"start\": 1.0, \"duration\": 1.0, \"velocity\": 80}, \n  {\"note\": 67, \"start\": 2.0, \"duration\": 1.0, \"velocity\": 80}\n]\n</parameter>\n<parameter name=\"description\">Simple C major triad</parameter>\n</invoke>\n</function_calls>"
})";

        LLMResponse response;
        response.result = realResponse;
        response.success = true;

        auto results = ResponseParser::parseStructuredResponse(response, "Anthropic",
                                                               "generate_musical_sequence");

        // Should find the XML function call
        REQUIRE(results.size() >= 1);

        // Find the result that contains our expected data
        bool foundExpectedResult = false;
        for (const auto& result : results) {
            if (result.data.is_object() && result.data.contains("sequence_data")) {
                foundExpectedResult = true;

                // Verify the parsed data
                REQUIRE(result.data["sequence_data"].is_array());
                REQUIRE(result.data["sequence_data"].size() == 3);

                // Check first note
                auto firstNote = result.data["sequence_data"][0];
                REQUIRE(firstNote["note"] == 60);
                REQUIRE(firstNote["start"] == 0.0);
                REQUIRE(firstNote["duration"] == 1.0);
                REQUIRE(firstNote["velocity"] == 80);

                // Check second note
                auto secondNote = result.data["sequence_data"][1];
                REQUIRE(secondNote["note"] == 64);
                REQUIRE(secondNote["start"] == 1.0);
                REQUIRE(secondNote["duration"] == 1.0);
                REQUIRE(secondNote["velocity"] == 80);

                // Check third note
                auto thirdNote = result.data["sequence_data"][2];
                REQUIRE(thirdNote["note"] == 67);
                REQUIRE(thirdNote["start"] == 2.0);
                REQUIRE(thirdNote["duration"] == 1.0);
                REQUIRE(thirdNote["velocity"] == 80);

                // Check description
                REQUIRE(result.data["description"] == "Simple C major triad");

                break;
            }
        }

        REQUIRE(foundExpectedResult);
    }
}

TEST_CASE("ResponseParser Anthropic XML parsing", "[responseparser][anthropic][xml]") {
    SECTION("Parse basic XML function call") {
        std::string xmlResponse = R"(
<function_calls>
<invoke name="test_function">
<parameter name="data">{"key": "value"}</parameter>
</invoke>
</function_calls>
)";

        LLMResponse response;
        response.result = xmlResponse;
        response.success = true;

        auto results = ResponseParser::parseAnthropicXmlResponse(response);
        REQUIRE(results.size() >= 1);

        bool foundResult = false;
        for (const auto& result : results) {
            if (result.data.contains("key") && result.data["key"] == "value") {
                foundResult = true;
                break;
            }
        }
        REQUIRE(foundResult);
    }

    SECTION("Parse XML with array parameter") {
        std::string xmlResponse = R"(
<function_calls>
<invoke name="generate_sequence">
<parameter name="notes">[{"note": 60, "start": 0.0}, {"note": 64, "start": 1.0}]</parameter>
<parameter name="description">Test sequence</parameter>
</invoke>
</function_calls>
)";

        LLMResponse response;
        response.result = xmlResponse;
        response.success = true;

        auto results = ResponseParser::parseAnthropicXmlResponse(response);
        REQUIRE(results.size() >= 1);

        bool foundResult = false;
        for (const auto& result : results) {
            if (result.data.contains("notes") && result.data["notes"].is_array()) {
                REQUIRE(result.data["notes"].size() == 2);
                REQUIRE(result.data["notes"][0]["note"] == 60);
                REQUIRE(result.data["notes"][1]["note"] == 64);
                REQUIRE(result.data["description"] == "Test sequence");
                foundResult = true;
                break;
            }
        }
        REQUIRE(foundResult);
    }

    SECTION("Parse multiple function calls") {
        std::string xmlResponse = R"(
<function_calls>
<invoke name="function1">
<parameter name="data1">{"value": 1}</parameter>
</invoke>
</function_calls>

<function_calls>
<invoke name="function2">
<parameter name="data2">{"value": 2}</parameter>
</invoke>
</function_calls>
)";

        LLMResponse response;
        response.result = xmlResponse;
        response.success = true;

        auto results = ResponseParser::parseAnthropicXmlResponse(response);
        REQUIRE(results.size() >= 2);
    }
}

TEST_CASE("ResponseParser OpenAI JSON parsing", "[responseparser][openai][json]") {
    SECTION("Parse markdown-fenced JSON") {
        std::string jsonResponse = R"(
Here's the musical sequence:

```json
{
  "sequences": [
    {
      "description": "C major scale",
      "notes": [
        {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 80},
        {"note": 62, "start": 1.0, "duration": 1.0, "velocity": 80}
      ]
    }
  ]
}
```
)";

        LLMResponse response;
        response.result = jsonResponse;
        response.success = true;

        auto results = ResponseParser::parseOpenAIJsonResponse(response);
        REQUIRE(results.size() >= 1);

        bool foundResult = false;
        for (const auto& result : results) {
            if (result.data.contains("sequences") && result.data["sequences"].is_array()) {
                auto seq = result.data["sequences"][0];
                REQUIRE(seq["description"] == "C major scale");
                REQUIRE(seq["notes"].is_array());
                REQUIRE(seq["notes"].size() == 2);
                foundResult = true;
                break;
            }
        }
        REQUIRE(foundResult);
    }

    SECTION("Parse plain JSON array") {
        std::string jsonResponse = R"(
[
  {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 80},
  {"note": 64, "start": 1.0, "duration": 1.0, "velocity": 80}
]
)";

        LLMResponse response;
        response.result = jsonResponse;
        response.success = true;

        auto results = ResponseParser::parseJsonArrayFromText(response.result);
        REQUIRE(results.size() >= 1);

        auto result = results[0];
        REQUIRE(result.data.is_array());
        REQUIRE(result.data.size() == 2);
        REQUIRE(result.data[0]["note"] == 60);
        REQUIRE(result.data[1]["note"] == 64);
    }
}

TEST_CASE("ResponseParser provider detection", "[responseparser][detection]") {
    SECTION("Detect Anthropic XML format") {
        std::string anthropicText = "Here's the result:\n<function_calls>\n<invoke name=\"test\">";
        REQUIRE(ResponseParser::looksLikeAnthropicXml(anthropicText));
    }

    SECTION("Detect JSON format") {
        std::string jsonText = "```json\n{\"key\": \"value\"}\n```";
        REQUIRE(ResponseParser::containsMarkdownFencedJson(jsonText));
    }

    SECTION("Extract text from response") {
        LLMResponse response;
        response.result = R"({"text": "Hello world", "other": "data"})";
        response.success = true;

        std::string text = ResponseParser::extractTextFromResponse(response);
        REQUIRE(text == "Hello world");
    }

    SECTION("Extract text from plain string response") {
        LLMResponse response;
        response.result = "Plain text response";
        response.success = true;

        std::string text = ResponseParser::extractTextFromResponse(response);
        REQUIRE(text == "Plain text response");
    }
}

TEST_CASE("ResponseParser JSON utilities", "[responseparser][json][utilities]") {
    SECTION("Extract balanced JSON array") {
        std::string text = "Here is data: [1, 2, 3] and more text";
        auto extracted = ResponseParser::extractBalancedJsonArray(text);
        REQUIRE(extracted == "[1, 2, 3]");
    }

    SECTION("Extract JSON object") {
        std::string text = "Result: {\"key\": \"value\", \"nested\": {\"inner\": 1}} end";
        auto extracted = ResponseParser::extractBalancedJsonObject(text);
        REQUIRE(extracted == R"({"key": "value", "nested": {"inner": 1}})");
    }

    SECTION("Strip markdown fences") {
        std::string fenced = "```json\n{\"test\": true}\n```";
        auto stripped = ResponseParser::stripMarkdownFences(fenced);
        REQUIRE(stripped == R"({"test": true})");
    }

    SECTION("Strip markdown fences with language") {
        std::string fenced = "```javascript\nconsole.log('test');\n```";
        auto stripped = ResponseParser::stripMarkdownFences(fenced);
        REQUIRE(stripped == "console.log('test');");
    }
}
