#include <iostream>

#include "core/ResponseParser.h"
#include "llmcpp.h"

int main() {
    // Exact same input as failing test
    std::string text =
        "I'll create several jazzy chord progressions...\n\n<musical_aideas-musical_sequence>\n";
    text += R"([
      {
        "description": "Test progression",
        "sequence": [
          {"note": 60, "start": 0.0, "duration": 1.0, "velocity": 90},
          {"note": 64, "start": 1.0, "duration": 1.0, "velocity": 90},
          {"note": 67, "start": 2.0, "duration": 2.0, "velocity": 90}
        ]
      }
    ])";

    // Create response exactly like the test
    llmcpp::LLMResponse response;
    response.success = true;
    response.result = nlohmann::json{{"text", text}};

    // Call parseStructuredResponse exactly like aideas does
    auto results = llmcpp::ResponseParser::parseStructuredResponse(
        response, "Anthropic", "musical_aideas-musical_sequence");

    std::cout << "Results count: " << results.size() << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "Result " << i << ":" << std::endl;
        std::cout << "  source: " << results[i].source << std::endl;
        std::cout << "  description: " << results[i].description << std::endl;
        std::cout << "  data: " << results[i].data.dump(2) << std::endl;
    }

    return 0;
}
