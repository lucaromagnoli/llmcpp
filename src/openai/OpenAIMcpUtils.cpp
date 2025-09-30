#include "openai/OpenAIMcpUtils.h"

namespace OpenAI {
namespace McpUtils {

std::vector<McpToolCall> extractMcpCalls(const ResponsesResponse& response) {
    std::vector<McpToolCall> calls;

    if (response.output.empty()) {
        return calls;
    }

    for (const auto& item : response.output) {
        if (!item.is_object() || !item.contains("type")) {
            continue;
        }

        std::string type = item["type"].get<std::string>();

        // Look for mcp_call type items
        if (type == "mcp_call") {
            McpToolCall call;

            if (item.contains("id")) {
                call.id = item["id"].get<std::string>();
            }

            if (item.contains("name")) {
                call.tool = item["name"].get<std::string>();
            }

            if (item.contains("input")) {
                call.input = item["input"];
            }

            // Check for output
            if (item.contains("output")) {
                call.output = item["output"];
                call.success = true;
            }

            // Check for error
            if (item.contains("error")) {
                call.error = item["error"].is_string() ? item["error"].get<std::string>()
                                                       : item["error"].dump();
                call.success = false;
            }

            calls.push_back(call);
        }
    }

    return calls;
}

bool wasToolCalled(const ResponsesResponse& response, const std::string& toolName) {
    auto calls = extractMcpCalls(response);
    for (const auto& call : calls) {
        if (call.tool == toolName) {
            return true;
        }
    }
    return false;
}

std::optional<json> getToolOutput(const ResponsesResponse& response, const std::string& toolName) {
    auto calls = extractMcpCalls(response);
    for (const auto& call : calls) {
        if (call.tool == toolName && call.output.has_value()) {
            return call.output.value();
        }
    }
    return std::nullopt;
}

std::vector<json> getAllToolOutputs(const ResponsesResponse& response,
                                    const std::string& toolName) {
    std::vector<json> outputs;
    auto calls = extractMcpCalls(response);

    for (const auto& call : calls) {
        if (call.tool == toolName && call.output.has_value()) {
            outputs.push_back(call.output.value());
        }
    }

    return outputs;
}

bool wereMcpToolsListed(const ResponsesResponse& response) {
    if (response.output.empty()) {
        return false;
    }

    for (const auto& item : response.output) {
        if (item.is_object() && item.contains("type")) {
            std::string type = item["type"].get<std::string>();
            if (type == "mcp_list_tools") {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> getAvailableMcpTools(const ResponsesResponse& response) {
    std::vector<std::string> tools;

    if (response.output.empty()) {
        return tools;
    }

    for (const auto& item : response.output) {
        if (!item.is_object() || !item.contains("type")) {
            continue;
        }

        std::string type = item["type"].get<std::string>();

        if (type == "mcp_list_tools" && item.contains("tools") && item["tools"].is_array()) {
            for (const auto& tool : item["tools"]) {
                if (tool.is_object() && tool.contains("name")) {
                    tools.push_back(tool["name"].get<std::string>());
                } else if (tool.is_string()) {
                    tools.push_back(tool.get<std::string>());
                }
            }
        }
    }

    return tools;
}

bool wereAllToolsCalled(const ResponsesResponse& response,
                        const std::vector<std::string>& expectedTools) {
    auto calls = extractMcpCalls(response);

    for (const auto& expectedTool : expectedTools) {
        bool found = false;
        for (const auto& call : calls) {
            if (call.tool == expectedTool) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

json getMcpUsageStats(const ResponsesResponse& response) {
    auto calls = extractMcpCalls(response);

    int totalCalls = static_cast<int>(calls.size());
    int successfulCalls = 0;
    int failedCalls = 0;
    std::vector<std::string> toolsUsed;

    for (const auto& call : calls) {
        if (call.success) {
            successfulCalls++;
        } else {
            failedCalls++;
        }

        // Track unique tools used
        if (std::find(toolsUsed.begin(), toolsUsed.end(), call.tool) == toolsUsed.end()) {
            toolsUsed.push_back(call.tool);
        }
    }

    return json{{"total_calls", totalCalls},
                {"successful_calls", successfulCalls},
                {"failed_calls", failedCalls},
                {"tools_used", toolsUsed}};
}

}  // namespace McpUtils
}  // namespace OpenAI
