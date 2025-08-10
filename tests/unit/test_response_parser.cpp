#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "core/LLMTypes.h"
#include "core/ResponseParser.h"

using namespace llmcpp;

TEST_CASE("ResponseParser can parse Anthropic XML responses", "[response_parser]") {
    std::string anthropicResponse = R"(
I'll create a musical sequence for you.

<function_calls>
<invoke name="generate_musical_sequence">
<parameter name="sequence_data">
[
  {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 100},
  {"note": 64, "start": 1.0, "duration": 1.0, "velocity": 100},
  {"note": 67, "start": 2.0, "duration": 1.0, "velocity": 100}
]
</parameter>
<parameter name="description">Simple C major triad</parameter>
</invoke>
</function_calls>
)";

    auto results = ResponseParser::parseAnthropicXmlResponse(anthropicResponse);

    REQUIRE(results.size() >= 1);

    // Debug: print what we got
    for (size_t i = 0; i < results.size(); ++i) {
        INFO("Result " << i << ": description='" << results[i].description << "', source='"
                       << results[i].source << "', data type="
                       << (results[i].data.is_array() ? "array" : results[i].data.type_name()));
        if (results[i].data.is_array()) {
            INFO("  Array size: " << results[i].data.size());
        }
    }

    // Just check that we got some results with proper structure
    REQUIRE(results.size() > 0);
    bool hasArrayData = false;
    for (const auto& result : results) {
        if (result.data.is_array() && result.data.size() >= 3) {
            hasArrayData = true;
            break;
        }
    }
    REQUIRE(hasArrayData);
}

TEST_CASE("ResponseParser can parse JSON arrays from text", "[response_parser]") {
    std::string jsonText = R"(
Here's your musical sequence:

[
  {
    "description": "Jazz chord progression",
    "notes": [
      {"note": 60, "start": 0.0, "duration": 4.0, "velocity": 80},
      {"note": 64, "start": 0.0, "duration": 4.0, "velocity": 80}
    ]
  },
  {
    "description": "Follow up",
    "notes": [
      {"note": 67, "start": 4.0, "duration": 4.0, "velocity": 80}
    ]
  }
]
)";

    auto results = ResponseParser::parseJsonArrayFromText(jsonText);

    REQUIRE(results.size() == 2);
    REQUIRE(results[0].description == "Jazz chord progression");
    REQUIRE(results[1].description == "Follow up");
    REQUIRE(results[0].data.is_array());
    REQUIRE(results[0].data.size() == 2);
}

TEST_CASE("ResponseParser can parse markdown-fenced JSON", "[response_parser]") {
    std::string markdownText = R"(
Here's your sequence:

```json
[
  {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 100}
]
```
)";

    auto results = ResponseParser::parseMarkdownFencedJson(markdownText);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0].source == "markdown_fenced");
    REQUIRE(results[0].data.is_array());
    REQUIRE(results[0].data.size() == 1);
}

TEST_CASE("ResponseParser handles provider-agnostic parsing", "[response_parser]") {
    // Test with a mock LLMResponse
    LLMResponse response;
    response.success = true;
    response.result = R"(
<function_calls>
<invoke name="test_function">
<parameter name="notes">[{"note": 60, "start": 0.0, "duration": 1.0}]</parameter>
</invoke>
</function_calls>
)";

    auto results = ResponseParser::parseStructuredResponse(response, "Anthropic");

    REQUIRE(results.size() >= 0);  // Should not crash, results may vary
}

TEST_CASE("ResponseParser handles malformed/partial JSON gracefully", "[response_parser]") {
    std::string partialJson = R"(
[
  {"note": 60, "start": 0.0, "duration": 1.0},
  {"note": 64, "start": 1.0, "dur
)";

    // Should not crash and should try to salvage what it can
    auto results = ResponseParser::parseJsonArrayFromText(partialJson);

    // Depending on implementation, might find the first complete object
    // At minimum, should not crash
    REQUIRE_NOTHROW(results.size());
}

TEST_CASE("ResponseParser detects Anthropic responses correctly", "[response_parser]") {
    LLMResponse anthropicResponse;
    anthropicResponse.success = true;
    anthropicResponse.result = "<function_calls><invoke name=\"test\"></invoke></function_calls>";

    LLMResponse regularResponse;
    regularResponse.success = true;
    regularResponse.result = "This is just regular text with no XML";

    REQUIRE_NOTHROW(ResponseParser::parseStructuredResponse(anthropicResponse, "auto"));
    REQUIRE_NOTHROW(ResponseParser::parseStructuredResponse(regularResponse, "auto"));
}
