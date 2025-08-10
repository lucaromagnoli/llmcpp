#include "core/ResponseParser.h"

#include <algorithm>
#include <iostream>
#include <regex>

namespace llmcpp {

std::vector<ParsedResult> ResponseParser::parseStructuredResponse(const LLMResponse& response,
                                                                  const std::string& providerName,
                                                                  const std::string& functionName) {
    if (!response.success) {
        return {};
    }

    // Extract text from response
    std::string responseText =
        response.result.is_string() ? response.result.get<std::string>() : response.result.dump();

    // Provider-specific parsing
    if (providerName == "Anthropic" || providerName == "anthropic") {
        return parseAnthropicXmlResponse(responseText, functionName);
    } else if (providerName == "OpenAI" || providerName == "openai") {
        return parseOpenAIJsonResponse(response);
    }

    // Fallback: try to detect format automatically
    if (isAnthropicResponse(responseText)) {
        return parseAnthropicXmlResponse(responseText, functionName);
    } else {
        return parseOpenAIJsonResponse(response);
    }
}

std::vector<ParsedResult> ResponseParser::parseAnthropicXmlResponse(
    const std::string& text, const std::string& functionName) {
    (void)functionName;  // TODO: Use functionName for filtering
    std::vector<ParsedResult> results;

    // Strip markdown fences first
    std::string cleanText = stripMarkdownFences(text);

    // Extract XML content
    std::string xmlContent = extractXmlContent(cleanText);
    if (xmlContent.empty()) {
        // Fallback to JSON array parsing if no XML found
        return parseJsonArrayFromText(cleanText);
    }

    // Parse XML function calls
    auto xmlResults = parseXmlFunctionCalls(xmlContent);
    if (!xmlResults.empty()) {
        for (auto& result : xmlResults) {
            result.source = "anthropic_xml";
        }
        return xmlResults;
    }

    // Final fallback
    return parseJsonArrayFromText(cleanText);
}

std::vector<ParsedResult> ResponseParser::parseOpenAIJsonResponse(const LLMResponse& response) {
    std::vector<ParsedResult> results;

    try {
        // Handle structured JSON response
        if (response.result.is_object()) {
            // Handle Responses API format
            if (response.result.contains("choices") && response.result["choices"].is_array()) {
                auto choices = response.result["choices"];
                if (!choices.empty() && choices[0].contains("message")) {
                    auto message = choices[0]["message"];

                    // Check for tool calls
                    if (message.contains("tool_calls") && message["tool_calls"].is_array()) {
                        for (const auto& toolCall : message["tool_calls"]) {
                            if (toolCall.contains("function") &&
                                toolCall["function"].contains("arguments")) {
                                std::string argsStr = toolCall["function"]["arguments"];
                                auto args = nlohmann::json::parse(argsStr);
                                results.emplace_back("", args, "openai_tool_call");
                            }
                        }
                        return results;
                    }

                    // Check for direct content
                    if (message.contains("content")) {
                        std::string content = message["content"];
                        return parseJsonArrayFromText(content);
                    }
                }
            }
        }

        // Fallback to parsing as text
        std::string responseText = response.result.is_string() ? response.result.get<std::string>()
                                                               : response.result.dump();
        return parseJsonArrayFromText(responseText);

    } catch (const std::exception& e) {
        // If JSON parsing fails, try text parsing
        std::string responseText = response.result.is_string() ? response.result.get<std::string>()
                                                               : response.result.dump();
        return parseJsonArrayFromText(responseText);
    }
}

std::vector<ParsedResult> ResponseParser::parseJsonArrayFromText(const std::string& text) {
    std::vector<ParsedResult> results;

    // First try markdown-fenced JSON
    auto markdownResults = parseMarkdownFencedJson(text);
    if (!markdownResults.empty()) {
        return markdownResults;
    }

    // Find first JSON array in text
    size_t arrayStart = text.find('[');
    if (arrayStart == std::string::npos) {
        return results;
    }

    std::string arrayText = extractBalancedJsonArray(text, arrayStart);
    if (arrayText.empty()) {
        // Try to salvage partial JSON
        auto objects = salvageJsonObjects(text);
        if (!objects.empty()) {
            arrayText = rebuildJsonArray(objects);
        }
    }

    if (arrayText.empty()) {
        return results;
    }

    try {
        auto jsonArray = nlohmann::json::parse(arrayText);
        if (!jsonArray.is_array()) {
            return results;
        }

        // Parse array items
        for (const auto& item : jsonArray) {
            if (item.is_object()) {
                std::string description = item.value("description", "");

                // Look for notes/sequence data
                if (item.contains("notes")) {
                    results.emplace_back(description, item["notes"], "json_array_notes");
                } else if (item.contains("sequence")) {
                    results.emplace_back(description, item["sequence"], "json_array_sequence");
                } else {
                    // Include the whole object as data
                    results.emplace_back(description, item, "json_array_object");
                }
            } else {
                // Handle array of primitives
                results.emplace_back("", item, "json_array_primitive");
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    return results;
}

std::vector<ParsedResult> ResponseParser::parseMarkdownFencedJson(const std::string& text) {
    std::vector<ParsedResult> results;

    std::regex jsonBlockRegex(R"(```(?:json)?\s*([\s\S]*?)```)");
    std::sregex_iterator iter(text.begin(), text.end(), jsonBlockRegex);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::string jsonText = (*iter)[1].str();
        jsonText = normalizeWhitespace(jsonText);

        try {
            auto json = nlohmann::json::parse(jsonText);
            results.emplace_back("", json, "markdown_fenced");
        } catch (const std::exception& e) {
            // Continue to next block if this one fails
            continue;
        }
    }

    return results;
}

// Helper method implementations

std::string ResponseParser::extractXmlContent(const std::string& text) {
    size_t xmlStart = text.find('<');
    if (xmlStart == std::string::npos) {
        return "";
    }

    // Wrap in root element for easier parsing
    return "<root>" + text.substr(xmlStart) + "</root>";
}

std::vector<ParsedResult> ResponseParser::parseXmlFunctionCalls(const std::string& xmlText) {
    std::vector<ParsedResult> results;

    // Simple XML parsing - look for function_calls and parameters
    std::regex functionCallRegex("<invoke\\s+name=\"([^\"]+)\">(.*?)</invoke>");
    std::sregex_iterator iter(xmlText.begin(), xmlText.end(), functionCallRegex);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::string functionName = (*iter)[1].str();
        std::string parameters = (*iter)[2].str();

        // Extract parameter values
        std::string description = extractParameterValue(parameters, "description");
        std::string sequenceData = extractParameterValue(parameters, "sequence_data");
        std::string sequences = extractParameterValue(parameters, "sequences");
        std::string notes = extractParameterValue(parameters, "notes");

        // Try to parse JSON data from parameters
        if (!sequenceData.empty()) {
            try {
                auto json = nlohmann::json::parse(sequenceData);
                results.emplace_back(description, json, "xml_sequence_data");
            } catch (const std::exception& e) {
                // Skip invalid JSON
            }
        } else if (!sequences.empty()) {
            try {
                auto json = nlohmann::json::parse(sequences);
                results.emplace_back(description, json, "xml_sequences");
            } catch (const std::exception& e) {
                // Skip invalid JSON
            }
        } else if (!notes.empty()) {
            try {
                auto json = nlohmann::json::parse(notes);
                results.emplace_back(description, json, "xml_notes");
            } catch (const std::exception& e) {
                // Skip invalid JSON
            }
        }
    }

    return results;
}

std::string ResponseParser::extractParameterValue(const std::string& xmlText,
                                                  const std::string& paramName) {
    std::regex paramRegex("<parameter\\s+name=\"" + paramName + "\"[^>]*>(.*?)</parameter>");
    std::smatch match;

    if (std::regex_search(xmlText, match, paramRegex)) {
        return match[1].str();
    }

    return "";
}

std::string ResponseParser::extractBalancedJsonArray(const std::string& text, size_t startPos) {
    if (startPos >= text.length() || text[startPos] != '[') {
        return "";
    }

    int depth = 0;
    bool inString = false;
    bool escape = false;

    for (size_t i = startPos; i < text.length(); ++i) {
        char c = text[i];

        if (inString) {
            if (escape) {
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == '"') {
                inString = false;
            }
        } else {
            if (c == '"') {
                inString = true;
            } else if (c == '[') {
                depth++;
            } else if (c == ']') {
                depth--;
                if (depth == 0) {
                    return text.substr(startPos, i - startPos + 1);
                }
            }
        }
    }

    return "";  // Unbalanced
}

std::string ResponseParser::extractBalancedJsonObject(const std::string& text, size_t startPos) {
    if (startPos >= text.length() || text[startPos] != '{') {
        return "";
    }

    int depth = 0;
    bool inString = false;
    bool escape = false;

    for (size_t i = startPos; i < text.length(); ++i) {
        char c = text[i];

        if (inString) {
            if (escape) {
                escape = false;
            } else if (c == '\\') {
                escape = true;
            } else if (c == '"') {
                inString = false;
            }
        } else {
            if (c == '"') {
                inString = true;
            } else if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
                if (depth == 0) {
                    return text.substr(startPos, i - startPos + 1);
                }
            }
        }
    }

    return "";  // Unbalanced
}

std::vector<std::string> ResponseParser::salvageJsonObjects(const std::string& text) {
    std::vector<std::string> objects;

    for (size_t pos = 0; pos < text.length(); ++pos) {
        if (text[pos] == '{') {
            std::string obj = extractBalancedJsonObject(text, pos);
            if (!obj.empty()) {
                objects.push_back(obj);
                pos += obj.length() - 1;  // Skip past this object
            }
        }
    }

    return objects;
}

std::string ResponseParser::rebuildJsonArray(const std::vector<std::string>& objects) {
    if (objects.empty()) {
        return "";
    }

    std::string result = "[";
    for (size_t i = 0; i < objects.size(); ++i) {
        if (i > 0) result += ",";
        result += objects[i];
    }
    result += "]";

    return result;
}

std::string ResponseParser::stripMarkdownFences(const std::string& text) {
    std::string result = text;

    // Remove ```json and ``` fences
    result = std::regex_replace(result, std::regex(R"(```(?:json)?\s*)"), "");
    result = std::regex_replace(result, std::regex(R"(\s*```)"), "");

    return result;
}

std::string ResponseParser::normalizeWhitespace(const std::string& text) {
    std::string result = text;

    // Remove leading/trailing whitespace
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);

    return result;
}

bool ResponseParser::isJsonArray(const std::string& text) {
    std::string trimmed = normalizeWhitespace(text);
    return !trimmed.empty() && trimmed[0] == '[' && trimmed.back() == ']';
}

bool ResponseParser::isJsonObject(const std::string& text) {
    std::string trimmed = normalizeWhitespace(text);
    return !trimmed.empty() && trimmed[0] == '{' && trimmed.back() == '}';
}

bool ResponseParser::isAnthropicResponse(const std::string& text) {
    return text.find("<function_calls>") != std::string::npos ||
           text.find("<invoke") != std::string::npos ||
           text.find("<parameter") != std::string::npos;
}

bool ResponseParser::isOpenAIResponse(const LLMResponse& response) {
    // This is a simple heuristic - could be improved
    try {
        if (response.result.is_object()) {
            return response.result.contains("choices") || response.result.contains("data");
        }
        return false;
    } catch (const std::exception& e) {
        return false;
    }
}

}  // namespace llmcpp
