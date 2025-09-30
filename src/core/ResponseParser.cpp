#include "core/ResponseParser.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <stdexcept>

namespace llmcpp {

std::vector<ParsedResult> ResponseParser::parseStructuredResponse(const LLMResponse& response,
                                                                  const std::string& /*providerName*/,
                                                                  const std::string& /*functionName*/) {
    if (!response.success) {
        return {};
    }

    // Just return the raw response - don't parse anything
    // Let aideas handle all parsing logic
    std::vector<ParsedResult> results;
    results.emplace_back("", response.result, "raw_response");
    return results;
}

std::vector<ParsedResult> ResponseParser::parseAnthropicXmlResponse(
    const std::string& text, const std::string& /*functionName*/) {
    // Anthropic returns structured JSON in "text" field, often wrapped in markdown fences
    try {
        auto jsonResponse = nlohmann::json::parse(text);
        if (jsonResponse.contains("text") && jsonResponse["text"].is_string()) {
            std::string content = jsonResponse["text"].get<std::string>();

            // Strip markdown fences if present
            content = stripMarkdownFences(content);

            auto contentJson = nlohmann::json::parse(content);
            std::vector<ParsedResult> results;
            results.emplace_back("", contentJson, "anthropic_structured");
            return results;
        }
    } catch (const std::exception& e) {
        // If parsing fails, return empty
    }
    return {};
}

std::vector<ParsedResult> ResponseParser::parseDirectFunctionTags(const std::string& text,
                                                                  const std::string& functionName) {
    std::vector<ParsedResult> results;

    // Require function name parameter
    if (functionName.empty()) {
        throw std::invalid_argument("parseDirectFunctionTags: functionName parameter is required");
    }

    // Look for function tags (with or without closing tags)
    std::string openTag = "<" + functionName + ">";
    std::string closeTag = "</" + functionName + ">";

    size_t start = text.find(openTag);
    if (start != std::string::npos) {
        start += openTag.length();

        // Look for closing tag first
        size_t end = text.find(closeTag, start);
        std::string jsonContent;

        if (end != std::string::npos) {
            // Found closing tag
            jsonContent = text.substr(start, end - start);
        } else {
            // No closing tag - extract JSON from the current position to end of text
            jsonContent = text.substr(start);

            // Try to extract just the JSON part by finding balanced brackets
            size_t firstBrace = jsonContent.find_first_of("[{");
            if (firstBrace != std::string::npos) {
                jsonContent = jsonContent.substr(firstBrace);

                // Find matching closing bracket/brace
                int depth = 0;
                char openChar = jsonContent[0];
                char closeChar = (openChar == '[') ? ']' : '}';
                size_t endPos = 0;

                for (size_t i = 0; i < jsonContent.length(); ++i) {
                    if (jsonContent[i] == openChar)
                        depth++;
                    else if (jsonContent[i] == closeChar) {
                        depth--;
                        if (depth == 0) {
                            endPos = i + 1;
                            break;
                        }
                    }
                }

                if (endPos > 0) {
                    jsonContent = jsonContent.substr(0, endPos);
                }
            }
        }

        // Trim whitespace
        jsonContent.erase(0, jsonContent.find_first_not_of(" \t\n\r"));
        jsonContent.erase(jsonContent.find_last_not_of(" \t\n\r") + 1);

        try {
            auto jsonData = nlohmann::json::parse(jsonContent);
            std::string description = "Function call: " + functionName;
            results.emplace_back(description, jsonData, "direct_function_tag");
        } catch (const std::exception& e) {
            // If not valid JSON, ignore
        }
    }

    return results;
}

std::vector<ParsedResult> ResponseParser::parseOpenAIJsonResponse(const LLMResponse& response) {
    std::vector<ParsedResult> results;

    std::cerr << "DEBUG: parseOpenAIJsonResponse called" << std::endl;
    std::cerr << "DEBUG: response.result.is_object() = " << response.result.is_object()
              << std::endl;
    std::cerr << "DEBUG: response.result type = " << response.result.type_name() << std::endl;
    std::cerr << "DEBUG: response.result dump = " << response.result.dump().substr(0, 200) << "..."
              << std::endl;

    // Handle structured JSON response - return it exactly as-is
    if (response.result.is_object()) {
        std::cerr << "DEBUG: Returning openai_structured" << std::endl;
        results.emplace_back("", response.result, "openai_structured");
        return results;
    }

    std::cerr << "DEBUG: Response not structured, returning empty" << std::endl;
    return results;
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

        // Return the entire array as a single result
        results.emplace_back("", jsonArray, "json_array_text");

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

    // Look for function_calls blocks first (use [\s\S] instead of . to match newlines)
    std::regex functionCallsRegex(R"(<function_calls>([\s\S]*?)</function_calls>)");
    std::sregex_iterator functionCallsIter(xmlText.begin(), xmlText.end(), functionCallsRegex);
    std::sregex_iterator functionCallsEnd;

    for (; functionCallsIter != functionCallsEnd; ++functionCallsIter) {
        std::string functionCallsContent = (*functionCallsIter)[1].str();

        // Look for invoke elements within function_calls (use [\s\S] instead of . to match
        // newlines)
        std::regex invokeRegex(
            R"(<invoke\s+name\s*=\s*[\"']([^\"']+)[\"']\s*>([\s\S]*?)</invoke>)");
        std::sregex_iterator invokeIter(functionCallsContent.begin(), functionCallsContent.end(),
                                        invokeRegex);
        std::sregex_iterator invokeEnd;

        for (; invokeIter != invokeEnd; ++invokeIter) {
            std::string functionName = (*invokeIter)[1].str();
            std::string parameters = (*invokeIter)[2].str();

            // Extract all parameters using a more generic approach
            std::regex allParamsRegex(
                R"(<parameter\s+name\s*=\s*[\"']([^\"']+)[\"']\s*>([\s\S]*?)</parameter>)");
            std::sregex_iterator paramIter(parameters.begin(), parameters.end(), allParamsRegex);
            std::sregex_iterator paramEnd;

            // Create result object with all parameters
            nlohmann::json resultData;
            std::string description;

            for (; paramIter != paramEnd; ++paramIter) {
                std::string paramName = (*paramIter)[1].str();
                std::string paramValue = (*paramIter)[2].str();

                // Trim whitespace
                paramValue.erase(0, paramValue.find_first_not_of(" \t\n\r"));
                paramValue.erase(paramValue.find_last_not_of(" \t\n\r") + 1);

                if (paramName == "description") {
                    description = paramValue;
                }

                // Try to parse as JSON, fallback to string
                try {
                    auto json = nlohmann::json::parse(paramValue);
                    resultData[paramName] = json;
                } catch (const std::exception& e) {
                    // Store as string if JSON parsing fails
                    resultData[paramName] = paramValue;
                }
            }

            // Add result if we found any data
            if (!resultData.empty()) {
                results.emplace_back(description, resultData, "xml_function_call");
            }
        }
    }

    return results;
}

std::string ResponseParser::extractParameterValue(const std::string& xmlText,
                                                  const std::string& paramName) {
    // Handle both formats: <parameter name="param_name">value</parameter> (use [\s\S] to match
    // newlines)
    std::string pattern =
        R"(<parameter\s+name\s*=\s*[\"'])" + paramName + R"([\"']\s*>([\s\S]*?)</parameter>)";
    std::regex paramRegex(pattern);
    std::smatch match;

    if (std::regex_search(xmlText, match, paramRegex)) {
        std::string value = match[1].str();
        // Trim whitespace
        value.erase(0, value.find_first_not_of(" \t\n\r"));
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
        return value;
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
           text.find("<parameter") != std::string::npos ||
           text.find_first_of('<') !=
               std::string::npos;  // Any XML-like tags suggest Anthropic format
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
