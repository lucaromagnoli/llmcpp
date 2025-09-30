#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "openai/OpenAITypes.h"

using json = nlohmann::json;

namespace OpenAI {

/// MCP tool call information extracted from response
struct McpToolCall {
    std::string id;
    std::string tool;
    json input;
    std::optional<json> output;
    std::optional<std::string> error;
    bool success = false;

    std::string toString() const {
        std::string result = "McpToolCall { id: " + id + ", tool: " + tool;
        result += ", success: " + std::string(success ? "true" : "false");
        if (error.has_value()) {
            result += ", error: " + error.value();
        }
        if (output.has_value()) {
            std::string preview = output.value().dump();
            if (preview.length() > 100) {
                preview = preview.substr(0, 100) + "...";
            }
            result += ", output_preview: " + preview;
        }
        result += " }";
        return result;
    }
};

/// Utility functions for parsing MCP tool outputs from OpenAI responses
namespace McpUtils {

/**
 * Extract all MCP tool calls from a ResponsesResponse
 * @param response The OpenAI Responses API response
 * @return Vector of McpToolCall structs with details about each tool invocation
 */
std::vector<McpToolCall> extractMcpCalls(const ResponsesResponse& response);

/**
 * Check if a specific MCP tool was called in the response
 * @param response The OpenAI Responses API response
 * @param toolName The name of the tool to check for
 * @return true if the tool was called at least once
 */
bool wasToolCalled(const ResponsesResponse& response, const std::string& toolName);

/**
 * Get the output from a specific MCP tool call
 * @param response The OpenAI Responses API response
 * @param toolName The name of the tool
 * @return The output JSON if found, std::nullopt otherwise
 */
std::optional<json> getToolOutput(const ResponsesResponse& response, const std::string& toolName);

/**
 * Get all outputs from a specific MCP tool (if called multiple times)
 * @param response The OpenAI Responses API response
 * @param toolName The name of the tool
 * @return Vector of output JSON objects
 */
std::vector<json> getAllToolOutputs(const ResponsesResponse& response, const std::string& toolName);

/**
 * Check if any MCP tools were listed in the response
 * @param response The OpenAI Responses API response
 * @return true if mcp_list_tools output item was found
 */
bool wereMcpToolsListed(const ResponsesResponse& response);

/**
 * Get the list of available MCP tools from mcp_list_tools output
 * @param response The OpenAI Responses API response
 * @return Vector of tool names that are available
 */
std::vector<std::string> getAvailableMcpTools(const ResponsesResponse& response);

/**
 * Check if all expected tools were called
 * @param response The OpenAI Responses API response
 * @param expectedTools Vector of tool names that should have been called
 * @return true if all expected tools were called
 */
bool wereAllToolsCalled(const ResponsesResponse& response,
                        const std::vector<std::string>& expectedTools);

/**
 * Get summary statistics about MCP tool usage
 * @param response The OpenAI Responses API response
 * @return JSON object with statistics (total_calls, successful_calls, failed_calls, tools_used)
 */
json getMcpUsageStats(const ResponsesResponse& response);

}  // namespace McpUtils
}  // namespace OpenAI
