#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "core/LLMTypes.h"

namespace llmcpp {

/**
 * @brief Represents a parsed structured result from an LLM response
 */
struct ParsedResult {
    std::string description;
    nlohmann::json data;
    std::string source;  // For debugging - indicates which parser was used

    ParsedResult(const std::string& desc, const nlohmann::json& d, const std::string& src = "")
        : description(desc), data(d), source(src) {}
};

/**
 * @brief Provider-agnostic response parser for structured LLM outputs
 *
 * Handles various response formats from different LLM providers:
 * - Anthropic XML function calls
 * - OpenAI JSON responses
 * - Markdown-fenced JSON
 * - Mixed formats and fallbacks
 */
class ResponseParser {
   public:
    /**
     * @brief Parse a structured response from any LLM provider
     * @param response The LLM response object
     * @param providerName Name of the LLM provider (e.g., "Anthropic", "OpenAI")
     * @param functionName Expected function name for structured outputs
     * @return Vector of parsed results
     */
    static std::vector<ParsedResult> parseStructuredResponse(const LLMResponse& response,
                                                             const std::string& providerName,
                                                             const std::string& functionName = "");

    /**
     * @brief Parse Anthropic XML function call responses
     * @param text Raw response text containing XML function calls
     * @param functionName Optional function name to filter for
     * @return Vector of parsed results
     */
    static std::vector<ParsedResult> parseAnthropicXmlResponse(
        const std::string& text, const std::string& functionName = "");

    /**
     * @brief Parse OpenAI JSON responses (both Completions and Responses API)
     * @param response The LLM response object
     * @return Vector of parsed results
     */
    static std::vector<ParsedResult> parseOpenAIJsonResponse(const LLMResponse& response);

    /**
     * @brief Parse JSON arrays from text (with fallback for partial/malformed JSON)
     * @param text Text containing JSON arrays
     * @return Vector of parsed results
     */
    static std::vector<ParsedResult> parseJsonArrayFromText(const std::string& text);

    /**
     * @brief Extract and parse markdown-fenced JSON blocks
     * @param text Text containing ```json code blocks
     * @return Vector of parsed results
     */
    static std::vector<ParsedResult> parseMarkdownFencedJson(const std::string& text);

   private:
    // Helper methods for XML parsing
    static std::string extractXmlContent(const std::string& text);
    static std::vector<ParsedResult> parseXmlFunctionCalls(const std::string& xmlText);
    static std::string extractParameterValue(const std::string& xmlText,
                                             const std::string& paramName);

    // Helper methods for JSON parsing
    static std::string extractBalancedJsonArray(const std::string& text, size_t startPos);
    static std::string extractBalancedJsonObject(const std::string& text, size_t startPos);
    static std::vector<std::string> salvageJsonObjects(const std::string& text);
    static std::string rebuildJsonArray(const std::vector<std::string>& objects);

    // Text processing helpers
    static std::string stripMarkdownFences(const std::string& text);
    static std::string normalizeWhitespace(const std::string& text);
    static bool isJsonArray(const std::string& text);
    static bool isJsonObject(const std::string& text);

    // Provider detection
    static bool isAnthropicResponse(const std::string& text);
    static bool isOpenAIResponse(const LLMResponse& response);
};

}  // namespace llmcpp
