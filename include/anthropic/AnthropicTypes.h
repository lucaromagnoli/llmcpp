#pragma once
#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "core/LLMTypes.h"

using json = nlohmann::json;

namespace Anthropic {

/**
 * Anthropic Claude model names as strongly typed enum
 * Based on official Anthropic documentation:
 * https://docs.anthropic.com/en/docs/about-claude/models/overview
 */
enum class Model {
    // Claude 4 series (Latest - 2025)
    CLAUDE_OPUS_4_1,  // claude-opus-4-1-20250805 - Most capable and intelligent model
    CLAUDE_OPUS_4,    // claude-opus-4-20250514 - Previous flagship model
    CLAUDE_SONNET_4,  // claude-sonnet-4-20250514 - High-performance model

    // Claude 3.7 series
    CLAUDE_SONNET_3_7,  // claude-3-7-sonnet-20250219 - High-performance with extended thinking

    // Claude 3.5 series
    CLAUDE_SONNET_3_5_V2,  // claude-3-5-sonnet-20241022 - Latest 3.5 Sonnet (upgraded)
    CLAUDE_SONNET_3_5,     // claude-3-5-sonnet-20240620 - Previous 3.5 Sonnet
    CLAUDE_HAIKU_3_5,      // claude-3-5-haiku-20241022 - Fastest model

    // Claude 3 series (Legacy)
    CLAUDE_OPUS_3,   // claude-3-opus-20240229 - Legacy opus (deprecated)
    CLAUDE_HAIKU_3,  // claude-3-haiku-20240307 - Fast and compact legacy model

    // Custom/Other
    Custom  // For custom model names not in this enum
};

/**
 * Convert Anthropic Model enum to API string
 */
inline std::string toString(Model model) {
    switch (model) {
        case Model::CLAUDE_OPUS_4_1:
            return "claude-opus-4-1-20250805";
        case Model::CLAUDE_OPUS_4:
            return "claude-opus-4-20250514";
        case Model::CLAUDE_SONNET_4:
            return "claude-sonnet-4-20250514";
        case Model::CLAUDE_SONNET_3_7:
            return "claude-3-7-sonnet-20250219";
        case Model::CLAUDE_SONNET_3_5_V2:
            return "claude-3-5-sonnet-20241022";
        case Model::CLAUDE_SONNET_3_5:
            return "claude-3-5-sonnet-20240620";
        case Model::CLAUDE_HAIKU_3_5:
            return "claude-3-5-haiku-20241022";
        case Model::CLAUDE_OPUS_3:
            return "claude-3-opus-20240229";
        case Model::CLAUDE_HAIKU_3:
            return "claude-3-haiku-20240307";
        case Model::Custom:
            return "custom";
        default:
            return "claude-3-5-sonnet-20241022";  // Default to latest stable 3.5 Sonnet
    }
}

/**
 * Convert API string to Anthropic Model enum
 */
inline Model modelFromString(const std::string& modelStr) {
    if (modelStr == "claude-opus-4-1-20250805") return Model::CLAUDE_OPUS_4_1;
    if (modelStr == "claude-opus-4-20250514") return Model::CLAUDE_OPUS_4;
    if (modelStr == "claude-sonnet-4-20250514") return Model::CLAUDE_SONNET_4;
    if (modelStr == "claude-3-7-sonnet-20250219") return Model::CLAUDE_SONNET_3_7;
    if (modelStr == "claude-3-5-sonnet-20241022") return Model::CLAUDE_SONNET_3_5_V2;
    if (modelStr == "claude-3-5-sonnet-20240620") return Model::CLAUDE_SONNET_3_5;
    if (modelStr == "claude-3-5-haiku-20241022") return Model::CLAUDE_HAIKU_3_5;
    if (modelStr == "claude-3-opus-20240229") return Model::CLAUDE_OPUS_3;
    if (modelStr == "claude-3-haiku-20240307") return Model::CLAUDE_HAIKU_3;

    // Support aliases
    if (modelStr == "claude-opus-4-1") return Model::CLAUDE_OPUS_4_1;
    if (modelStr == "claude-opus-4-0") return Model::CLAUDE_OPUS_4;
    if (modelStr == "claude-sonnet-4-0") return Model::CLAUDE_SONNET_4;
    if (modelStr == "claude-3-7-sonnet-latest") return Model::CLAUDE_SONNET_3_7;
    if (modelStr == "claude-3-5-sonnet-latest") return Model::CLAUDE_SONNET_3_5_V2;
    if (modelStr == "claude-3-5-haiku-latest") return Model::CLAUDE_HAIKU_3_5;

    return Model::Custom;
}

/**
 * Get all available Anthropic model strings
 */
inline std::vector<std::string> getAvailableModels() {
    return {// Latest Claude 4 models
            toString(Model::CLAUDE_OPUS_4_1), toString(Model::CLAUDE_OPUS_4),
            toString(Model::CLAUDE_SONNET_4),

            // Claude 3.7
            toString(Model::CLAUDE_SONNET_3_7),

            // Claude 3.5 models
            toString(Model::CLAUDE_SONNET_3_5_V2), toString(Model::CLAUDE_SONNET_3_5),
            toString(Model::CLAUDE_HAIKU_3_5),

            // Legacy Claude 3 models
            toString(Model::CLAUDE_OPUS_3), toString(Model::CLAUDE_HAIKU_3)};
}

/**
 * Anthropic message role
 */
enum class MessageRole { USER, ASSISTANT };

inline std::string toString(MessageRole role) {
    switch (role) {
        case MessageRole::USER:
            return "user";
        case MessageRole::ASSISTANT:
            return "assistant";
        default:
            return "user";
    }
}

/**
 * Anthropic message content (supports text, tool_use, and tool_result)
 */
struct MessageContent {
    std::string type = "text";
    std::string text;

    // For tool_use content
    std::string id;
    std::string name;
    json input;

    // For tool_result content
    std::string toolUseId;
    json content;
    bool isError = false;

    json toJson() const {
        if (type == "text") {
            return json{{"type", type}, {"text", text}};
        } else if (type == "tool_use") {
            return json{{"type", type}, {"id", id}, {"name", name}, {"input", input}};
        } else if (type == "tool_result") {
            json j = {{"type", type}, {"tool_use_id", toolUseId}, {"content", content}};
            if (isError) {
                j["is_error"] = true;
            }
            return j;
        }
        return json{{"type", type}, {"text", text}};
    }

    // Convenience constructors
    static MessageContent createText(const std::string& txt) {
        MessageContent content;
        content.type = "text";
        content.text = txt;
        return content;
    }

    static MessageContent createToolUse(const std::string& toolId, const std::string& toolName,
                                        const json& toolInput) {
        MessageContent content;
        content.type = "tool_use";
        content.id = toolId;
        content.name = toolName;
        content.input = toolInput;
        return content;
    }

    static MessageContent createToolResult(const std::string& useId, const json& result,
                                           bool error = false) {
        MessageContent content;
        content.type = "tool_result";
        content.toolUseId = useId;
        content.content = result;
        content.isError = error;
        return content;
    }
};

/**
 * Anthropic message
 */
struct Message {
    MessageRole role;
    std::vector<MessageContent> content;

    json toJson() const {
        json contentArray = json::array();
        for (const auto& c : content) {
            contentArray.push_back(c.toJson());
        }
        return json{{"role", toString(role)}, {"content", contentArray}};
    }
};

/**
 * Anthropic API configuration
 */
struct AnthropicConfig {
    std::string apiKey;
    std::string baseUrl = "https://api.anthropic.com";
    std::string anthropicVersion = "2025-05-14";
    Model defaultModel = Model::CLAUDE_SONNET_4;
    int timeoutSeconds = 30;

    AnthropicConfig() = default;
    explicit AnthropicConfig(const std::string& key) : apiKey(key) {}
};

/**
 * Tool definition for function calling
 */
struct Tool {
    std::string name;
    std::string description;
    json inputSchema;

    json toJson() const {
        return json{{"name", name}, {"description", description}, {"input_schema", inputSchema}};
    }
};

/**
 * Tool use content (when model calls a tool)
 */
struct ToolUse {
    std::string type = "tool_use";
    std::string id;
    std::string name;
    json input;

    json toJson() const {
        return json{{"type", type}, {"id", id}, {"name", name}, {"input", input}};
    }
};

/**
 * Tool result content (response to tool use)
 */
struct ToolResult {
    std::string type = "tool_result";
    std::string toolUseId;
    json content;
    bool isError = false;

    json toJson() const {
        json j = {{"type", type}, {"tool_use_id", toolUseId}};

        if (isError) {
            j["is_error"] = true;
        }

        j["content"] = content;
        return j;
    }
};

/**
 * Anthropic Messages API request
 */
struct MessagesRequest {
    std::string model;
    std::vector<Message> messages;
    std::optional<int> maxTokens;
    std::optional<double> temperature;
    std::optional<double> topP;
    std::optional<std::string> system;
    std::vector<std::string> stopSequences;
    std::vector<Tool> tools;                // Tool definitions for function calling
    std::optional<std::string> toolChoice;  // "auto", "any", or specific tool name

    json toJson() const {
        json j = {{"model", model}, {"messages", json::array()}};

        // Add messages
        for (const auto& msg : messages) {
            j["messages"].push_back(msg.toJson());
        }

        // Add optional parameters
        if (maxTokens.has_value()) {
            j["max_tokens"] = maxTokens.value();
        }
        if (temperature.has_value()) {
            j["temperature"] = temperature.value();
        }
        if (topP.has_value()) {
            j["top_p"] = topP.value();
        }
        if (system.has_value()) {
            j["system"] = system.value();
        }
        if (!stopSequences.empty()) {
            j["stop_sequences"] = stopSequences;
        }
        if (!tools.empty()) {
            j["tools"] = json::array();
            for (const auto& tool : tools) {
                j["tools"].push_back(tool.toJson());
            }
        }
        if (toolChoice.has_value()) {
            j["tool_choice"] = {"type", toolChoice.value()};
        }

        return j;
    }

    /**
     * Convert from common LLMRequest to Anthropic MessagesRequest
     */
    static MessagesRequest fromLLMRequest(const LLMRequest& request) {
        MessagesRequest req;
        req.model = request.config.model;

        // Add context messages first (chronological order)
        for (const auto& contextMsg : request.context) {
            Message msg;
            // Check if context message has role and content fields
            if (contextMsg.contains("role") && contextMsg.contains("content")) {
                std::string role = contextMsg["role"];
                if (role == "user") {
                    msg.role = MessageRole::USER;
                } else if (role == "assistant") {
                    msg.role = MessageRole::ASSISTANT;
                } else {
                    continue;  // Skip unknown roles
                }
                msg.content.push_back({.type = "text", .text = contextMsg["content"]});
                req.messages.push_back(msg);
            }
        }

        // Add main prompt as the final user message
        if (!request.prompt.empty()) {
            Message userMsg;
            userMsg.role = MessageRole::USER;
            userMsg.content.push_back({.type = "text", .text = request.prompt});
            req.messages.push_back(userMsg);
        }

        // Set optional parameters
        if (request.config.maxTokens.has_value()) {
            req.maxTokens = request.config.maxTokens.value();
        }
        if (request.config.temperature.has_value()) {
            req.temperature = request.config.temperature.value();
        }

        return req;
    }
};

/**
 * Anthropic usage information
 */
struct Usage {
    int inputTokens = 0;
    int outputTokens = 0;

    int totalTokens() const { return inputTokens + outputTokens; }
};

/**
 * Anthropic Messages API response
 */
struct MessagesResponse {
    std::string id;
    std::string type = "message";
    std::string role = "assistant";
    std::vector<MessageContent> content;
    std::string model;
    std::string stopReason;
    std::optional<std::string> stopSequence;
    Usage usage;

    /**
     * Convert to common LLMResponse
     */
    LLMResponse toLLMResponse() const {
        LLMResponse response;
        response.success = !content.empty();

        // Combine all text content and parse as JSON
        std::string fullText;
        for (const auto& c : content) {
            if (c.type == "text") {
                fullText += c.text;
            }
        }

        // Parse the response content directly as JSON
        response.result = json::parse(fullText);

        response.usage.inputTokens = usage.inputTokens;
        response.usage.outputTokens = usage.outputTokens;

        if (!response.success) {
            response.errorMessage = "No content in response";
        }

        return response;
    }

    /**
     * Parse from JSON response
     */
    static MessagesResponse fromJson(const json& j) {
        MessagesResponse response;

        if (j.contains("id") && !j["id"].is_null()) {
            response.id = j["id"];
        }
        if (j.contains("type") && !j["type"].is_null()) {
            response.type = j["type"];
        }
        if (j.contains("role") && !j["role"].is_null()) {
            response.role = j["role"];
        }
        if (j.contains("model") && !j["model"].is_null()) {
            response.model = j["model"];
        }
        if (j.contains("stop_reason") && !j["stop_reason"].is_null()) {
            response.stopReason = j["stop_reason"];
        }
        if (j.contains("stop_sequence") && !j["stop_sequence"].is_null()) {
            response.stopSequence = j["stop_sequence"];
        }

        // Parse content array
        if (j.contains("content") && j["content"].is_array()) {
            for (const auto& contentItem : j["content"]) {
                MessageContent content;
                if (contentItem.contains("type")) {
                    content.type = contentItem["type"];
                }
                if (contentItem.contains("text")) {
                    content.text = contentItem["text"];
                }
                // Parse tool_use content
                if (content.type == "tool_use") {
                    if (contentItem.contains("id")) {
                        content.id = contentItem["id"];
                    }
                    if (contentItem.contains("name")) {
                        content.name = contentItem["name"];
                    }
                    if (contentItem.contains("input")) {
                        content.input = contentItem["input"];
                    }
                }
                response.content.push_back(content);
            }
        }

        // Parse usage
        if (j.contains("usage")) {
            const auto& usageJson = j["usage"];
            if (usageJson.contains("input_tokens")) {
                response.usage.inputTokens = usageJson["input_tokens"];
            }
            if (usageJson.contains("output_tokens")) {
                response.usage.outputTokens = usageJson["output_tokens"];
            }
        }

        return response;
    }
};

}  // namespace Anthropic
