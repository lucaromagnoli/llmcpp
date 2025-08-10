#include "core/ResponseParser.h"

#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <stdexcept>

namespace llmcpp {

std::vector<ParsedResult> ResponseParser::parseStructuredResponse(const LLMResponse& response,
                                                                  const std::string& providerName,
                                                                  const std::string& functionName) {
    if (!response.success) {
        return {};
    }

    // Extract text from response
    std::string responseText;
    try {
        if (response.result.is_string()) {
            responseText = response.result.get<std::string>();
        } else if (response.result.is_object() && response.result.contains("text")) {
            responseText = response.result["text"].get<std::string>();
        } else {
            responseText = response.result.dump();
        }
    } catch (...) {
        responseText = response.result.dump();
    }

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
    std::vector<ParsedResult> results;

    // Strip markdown fences first
    std::string cleanText = stripMarkdownFences(text);

    // Try to parse standard XML function calls first (Anthropic format)
    auto xmlResults = parseXmlFunctionCalls(cleanText);
    if (!xmlResults.empty()) {
        for (auto& result : xmlResults) {
            result.source = "anthropic_xml";
        }
        return xmlResults;
    }

    // Try to parse direct function tags (e.g.,
    // <generate_musical_sequence>JSON</generate_musical_sequence>)
    auto directResults = parseDirectFunctionTags(cleanText, functionName);
    if (!directResults.empty()) {
        for (auto& result : directResults) {
            result.source = "direct_function_tag";
        }
        return directResults;
    }

    // If no XML found, fallback to JSON array parsing
    return parseJsonArrayFromText(cleanText);
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
