#pragma once
#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/LLMTypes.h"

using json = nlohmann::json;

namespace OpenAI {

/**
 * OpenAI model names as strongly typed enum
 * Provides type safety and IDE auto-completion for model selection
 */
enum class Model {
    // GPT-5 series (Latest - 2025)
    GPT_5,       // gpt-5 - Next-generation model
    GPT_5_Mini,  // gpt-5-mini - Smaller, cost-effective variant
    GPT_5_Nano,  // gpt-5-nano - Fastest and cheapest GPT-5 variant

    // O3 series (Latest - 2025)
    O3,       // o3 - Latest reasoning model
    O3_Mini,  // o3-mini - Cost-effective reasoning model

    // O1 series (2024-2025)
    O1,          // o1 - Advanced reasoning model
    O1_Mini,     // o1-mini - Cost-effective O1 model
    O1_Preview,  // o1-preview - Preview version
    O1_Pro,      // o1-pro - Professional version

    // O4 series (Latest - 2025)
    O4_Mini,                // o4-mini - Latest mini model
    O4_Mini_Deep_Research,  // o4-mini-deep-research - Research focused

    // GPT-4.1 series (Latest - April 2025)
    GPT_4_1,  // gpt-4.1 - Latest model with superior coding, instruction following, and structured
              // outputs
    GPT_4_1_Mini,  // gpt-4.1-mini - Balanced performance and cost, beats gpt-4o in many benchmarks
    GPT_4_1_Nano,  // gpt-4.1-nano - Fastest and cheapest option for simple tasks

    // GPT-4o series
    GPT_4o,       // gpt-4o - Good balance of performance and cost
    GPT_4o_Mini,  // gpt-4o-mini - Cost-effective for basic tasks

    // GPT-4.5 series (Latest - 2025)
    GPT_4_5,  // gpt-4.5-preview - Latest preview model

    // GPT-3.5 series (legacy)
    GPT_3_5_Turbo,  // gpt-3.5-turbo - Legacy model

    // Custom/Other
    Custom  // For custom model names not in this enum
};

/**
 * Convert OpenAI Model enum to API string
 */
inline std::string toString(Model model) {
    switch (model) {
        case Model::GPT_5:
            return "gpt-5";
        case Model::GPT_5_Mini:
            return "gpt-5-mini";
        case Model::GPT_5_Nano:
            return "gpt-5-nano";
        case Model::O3:
            return "o3";
        case Model::O3_Mini:
            return "o3-mini";
        case Model::O1:
            return "o1";
        case Model::O1_Mini:
            return "o1-mini";
        case Model::O1_Preview:
            return "o1-preview";
        case Model::O1_Pro:
            return "o1-pro";
        case Model::O4_Mini:
            return "o4-mini";
        case Model::O4_Mini_Deep_Research:
            return "o4-mini-deep-research";
        case Model::GPT_4_1:
            return "gpt-4.1";
        case Model::GPT_4_1_Mini:
            return "gpt-4.1-mini";
        case Model::GPT_4_1_Nano:
            return "gpt-4.1-nano";
        case Model::GPT_4o:
            return "gpt-4o";
        case Model::GPT_4o_Mini:
            return "gpt-4o-mini";
        case Model::GPT_4_5:
            return "gpt-4.5-preview";
        case Model::GPT_3_5_Turbo:
            return "gpt-3.5-turbo";
        case Model::Custom:
            return "custom";
    }
    return "unknown";
}

/**
 * Convert API string to OpenAI Model enum
 */
inline Model modelFromString(const std::string& modelStr) {
    if (modelStr == "gpt-5") return Model::GPT_5;
    if (modelStr == "gpt-5-mini") return Model::GPT_5_Mini;
    if (modelStr == "gpt-5-nano") return Model::GPT_5_Nano;
    if (modelStr == "o3") return Model::O3;
    if (modelStr == "o3-mini") return Model::O3_Mini;
    if (modelStr == "o1") return Model::O1;
    if (modelStr == "o1-mini") return Model::O1_Mini;
    if (modelStr == "o1-preview") return Model::O1_Preview;
    if (modelStr == "o1-pro") return Model::O1_Pro;
    if (modelStr == "o4-mini") return Model::O4_Mini;
    if (modelStr == "o4-mini-deep-research") return Model::O4_Mini_Deep_Research;
    if (modelStr == "gpt-4.1") return Model::GPT_4_1;
    if (modelStr == "gpt-4.1-mini") return Model::GPT_4_1_Mini;
    if (modelStr == "gpt-4.1-nano") return Model::GPT_4_1_Nano;
    if (modelStr == "gpt-4o") return Model::GPT_4o;
    if (modelStr == "gpt-4o-mini") return Model::GPT_4o_Mini;
    if (modelStr == "gpt-4.5-preview") return Model::GPT_4_5;
    if (modelStr == "gpt-3.5-turbo") return Model::GPT_3_5_Turbo;
    return Model::Custom;
}

/**
 * Check if model supports structured outputs via Responses API
 */
inline bool supportsStructuredOutputs(Model model) {
    switch (model) {
        case Model::GPT_5:
        case Model::GPT_5_Mini:
        case Model::GPT_5_Nano:
        case Model::O3:
        case Model::O3_Mini:
        case Model::O1:
        case Model::O1_Mini:
        case Model::O1_Preview:
        case Model::O1_Pro:
        case Model::O4_Mini:
        case Model::O4_Mini_Deep_Research:
        case Model::GPT_4_1:
        case Model::GPT_4_1_Mini:
        case Model::GPT_4_1_Nano:
        case Model::GPT_4o:
        case Model::GPT_4o_Mini:
        case Model::GPT_4_5:
            return true;
        case Model::GPT_3_5_Turbo:
        case Model::Custom:
            return false;
    }
    return false;
}

// OpenAI-specific simple message structure for convenience
struct Message {
    std::string role;  // "user", "assistant", "system"
    std::string content;

    json toJson() const { return json{{"role", role}, {"content", content}}; }

    static Message fromJson(const json& j) {
        Message msg;
        msg.role = j.at("role").get<std::string>();
        msg.content = j.at("content").get<std::string>();
        return msg;
    }
};

/**
 * Utility function for safely extracting JSON values with default fallbacks
 * Similar to Python's dict.get() method
 */
template <typename T>
T safeGetJson(const json& j, const std::string& key, const T& defaultValue = T{}) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return defaultValue;
}

/**
 * Utility function for safely extracting optional JSON values
 * Returns std::nullopt if key doesn't exist or is null
 */
template <typename T>
std::optional<T> safeGetOptionalJson(const json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return std::nullopt;
}

/**
 * Utility function for safely extracting required JSON values
 * Throws std::runtime_error with helpful message if key is missing or null
 */
template <typename T>
T safeGetRequiredJson(const json& j, const std::string& key) {
    if (!j.contains(key)) {
        throw std::runtime_error("Required JSON key '" + key + "' is missing");
    }
    if (j[key].is_null()) {
        throw std::runtime_error("Required JSON key '" + key + "' is null");
    }
    return j[key].get<T>();
}

/**
 * OpenAI Responses API Types (Modern Structured Output API)
 * Based on official OpenAI documentation: https://platform.openai.com/docs/api-reference/responses
 */

// Basic input types for the OpenAI API
struct TextInput {
    std::string text;
    std::string type = "input_text";

    json toJson() const { return json{{"text", text}, {"type", type}}; }

    static TextInput fromJson(const json& j) {
        TextInput input;
        input.text = j.at("text").get<std::string>();
        return input;
    }
};

struct ImageInput {
    std::string detail = "auto";  // "high", "low", "auto"
    std::string type = "input_image";
    std::optional<std::string> fileId;
    std::optional<std::string> imageUrl;

    json toJson() const {
        json j = {{"detail", detail}, {"type", type}};
        if (fileId) j["file_id"] = *fileId;
        if (imageUrl) j["image_url"] = *imageUrl;
        return j;
    }

    static ImageInput fromJson(const json& j) {
        ImageInput input;
        input.detail = j.value("detail", "auto");
        if (j.contains("file_id")) input.fileId = j["file_id"].get<std::string>();
        if (j.contains("image_url")) input.imageUrl = j["image_url"].get<std::string>();
        return input;
    }
};

struct FileInput {
    std::string type = "input_file";
    std::optional<std::string> fileData;
    std::optional<std::string> fileId;
    std::optional<std::string> filename;

    json toJson() const {
        json j = {{"type", type}};
        if (fileData) j["file_data"] = *fileData;
        if (fileId) j["file_id"] = *fileId;
        if (filename) j["filename"] = *filename;
        return j;
    }

    static FileInput fromJson(const json& j) {
        FileInput input;
        if (j.contains("file_data")) input.fileData = j["file_data"].get<std::string>();
        if (j.contains("file_id")) input.fileId = j["file_id"].get<std::string>();
        if (j.contains("filename")) input.filename = j["filename"].get<std::string>();
        return input;
    }
};

using InputContent = std::variant<TextInput, FileInput, ImageInput>;

// Message types for structured input
struct InputMessage {
    enum class Role { User, Assistant, System, Developer };

    std::variant<std::string, std::vector<InputContent>> content;
    Role role = Role::User;
    std::optional<std::string> type;  // Always "message" if provided

    static std::string roleToString(Role role) {
        switch (role) {
            case Role::User:
                return "user";
            case Role::Assistant:
                return "assistant";
            case Role::System:
                return "system";
            case Role::Developer:
                return "developer";
        }
        return "";
    }

    static Role stringToRole(const std::string& roleStr) {
        if (roleStr == "user") return Role::User;
        if (roleStr == "assistant") return Role::Assistant;
        if (roleStr == "system") return Role::System;
        if (roleStr == "developer") return Role::Developer;
        throw std::invalid_argument("Invalid role string: " + roleStr);
    }

    json toJson() const {
        json j;
        if (std::holds_alternative<std::string>(content)) {
            j["content"] = std::get<std::string>(content);
        } else {
            json contentArray = json::array();
            for (const auto& item : std::get<std::vector<InputContent>>(content)) {
                std::visit(
                    [&contentArray](const auto& input) { contentArray.push_back(input.toJson()); },
                    item);
            }
            j["content"] = contentArray;
        }
        j["role"] = roleToString(role);
        if (type) j["type"] = *type;
        return j;
    }

    static InputMessage fromJson(const json& j) {
        InputMessage msg;
        msg.role = stringToRole(j.at("role").get<std::string>());

        if (j["content"].is_string()) {
            msg.content = j["content"].get<std::string>();
        } else if (j["content"].is_array()) {
            std::vector<InputContent> contentList;
            for (const auto& item : j["content"]) {
                std::string itemType = item.at("type").get<std::string>();
                if (itemType == "input_text") {
                    contentList.push_back(TextInput::fromJson(item));
                } else if (itemType == "input_image") {
                    contentList.push_back(ImageInput::fromJson(item));
                } else if (itemType == "input_file") {
                    contentList.push_back(FileInput::fromJson(item));
                }
            }
            msg.content = contentList;
        }

        if (j.contains("type")) msg.type = j["type"].get<std::string>();
        return msg;
    }
};

// Input variants for the Responses API
struct ResponsesInput {
    enum class Type { String, ContentList } type;
    std::string textInput;
    std::vector<InputMessage> contentList;

    static ResponsesInput fromText(const std::string& text) {
        return ResponsesInput{Type::String, text, {}};
    }

    static ResponsesInput fromContentList(const std::vector<InputMessage>& messages) {
        return ResponsesInput{Type::ContentList, "", messages};
    }

    json toJson() const {
        if (type == Type::String) {
            return textInput;
        } else {
            json messageArray = json::array();
            std::transform(contentList.begin(), contentList.end(), std::back_inserter(messageArray),
                           [](const InputMessage& message) { return message.toJson(); });
            return messageArray;
        }
    }

    static ResponsesInput fromJson(const json& j) {
        if (j.is_string()) {
            return fromText(j.get<std::string>());
        } else if (j.is_array()) {
            std::vector<InputMessage> messages;
            std::transform(j.begin(), j.end(), std::back_inserter(messages),
                           [](const json& item) { return InputMessage::fromJson(item); });
            return fromContentList(messages);
        }
        throw std::invalid_argument("Invalid ResponsesInput format");
    }
};

// Text output configuration for structured responses
class TextOutputConfig {
   public:
    TextOutputConfig() = default;
    TextOutputConfig(std::string name, json schema, bool strict = true)
        : formatName(std::move(name)), formatSchema(std::move(schema)), isStrict(strict) {}

    json toJson() const {
        if (formatName.empty()) {
            return json{{"format", {{"type", "text"}}}};
        }

        json formatObj = {{"type", "json_schema"},
                          {"name", formatName},
                          {"schema", formatSchema},
                          {"strict", isStrict}};
        return json{{"format", formatObj}};
    }

    static TextOutputConfig fromJson(const json& j) {
        TextOutputConfig config;
        if (j.contains("format") && j["format"].contains("name")) {
            config.formatName = j["format"]["name"].get<std::string>();
            config.formatSchema = j["format"]["schema"];
            config.isStrict = j["format"].value("strict", true);
        }
        return config;
    }

   private:
    std::string formatName;
    json formatSchema;
    bool isStrict = true;
};

// Tool definitions
struct FunctionTool {
    std::string name;
    json parameters;
    bool strict = true;
    std::string type = "function";
    std::optional<std::string> description;

    json toJson() const {
        json j = {{"name", name}, {"parameters", parameters}, {"strict", strict}, {"type", type}};
        if (description) j["description"] = *description;
        return j;
    }

    static FunctionTool fromJson(const json& j) {
        FunctionTool tool;
        tool.name = j.at("name").get<std::string>();
        tool.parameters = j.at("parameters");
        tool.strict = j.value("strict", true);
        if (j.contains("description")) tool.description = j["description"].get<std::string>();
        return tool;
    }
};

struct WebSearchTool {
    std::string type = "web_search";
    std::optional<std::string> searchContextSize;  // "low", "medium", "high"
    std::optional<json> userLocation;
    std::optional<std::vector<std::string>> sites;

    json toJson() const {
        json j = {{"type", type}};
        if (searchContextSize) j["search_context_size"] = *searchContextSize;
        if (userLocation) j["user_location"] = *userLocation;
        if (sites) j["sites"] = *sites;
        return j;
    }

    static WebSearchTool fromJson(const json& j) {
        WebSearchTool tool;
        if (j.contains("search_context_size"))
            tool.searchContextSize = j["search_context_size"].get<std::string>();
        if (j.contains("user_location")) tool.userLocation = j["user_location"];
        if (j.contains("sites")) tool.sites = j["sites"].get<std::vector<std::string>>();
        return tool;
    }
};

struct FileSearchTool {
    std::string type = "file_search";
    std::vector<std::string> vectorStoreIds;
    std::optional<json> filters;
    std::optional<int> maxNumResults;  // 1-50
    std::optional<json> rankingOptions;

    json toJson() const {
        json j = {{"type", type}, {"vector_store_ids", vectorStoreIds}};
        if (filters) j["filters"] = *filters;
        if (maxNumResults) j["max_num_results"] = *maxNumResults;
        if (rankingOptions) j["ranking_options"] = *rankingOptions;
        return j;
    }

    static FileSearchTool fromJson(const json& j) {
        FileSearchTool tool;
        tool.vectorStoreIds = j.at("vector_store_ids").get<std::vector<std::string>>();
        if (j.contains("filters")) tool.filters = j["filters"];
        if (j.contains("max_num_results")) tool.maxNumResults = j["max_num_results"].get<int>();
        if (j.contains("ranking_options")) tool.rankingOptions = j["ranking_options"];
        return tool;
    }
};

struct CodeInterpreterTool {
    std::string type = "code_interpreter";

    json toJson() const { return json{{"type", type}}; }

    static CodeInterpreterTool fromJson(const json&) { return CodeInterpreterTool{}; }
};

struct ImageGenerationTool {
    std::string type = "image_generation";
    std::optional<int> partialImages;  // 1-3

    json toJson() const {
        json j = {{"type", type}};
        if (partialImages) j["partial_images"] = *partialImages;
        return j;
    }

    static ImageGenerationTool fromJson(const json& j) {
        ImageGenerationTool tool;
        if (j.contains("partial_images")) tool.partialImages = j["partial_images"].get<int>();
        return tool;
    }
};

struct McpTool {
    std::string type = "mcp";
    std::string serverLabel;
    std::string serverUrl;
    std::string requireApproval = "default";  // "always", "never", "default"
    std::optional<json> headers;              // For authentication

    json toJson() const {
        json j = {{"type", type},
                  {"server_label", serverLabel},
                  {"server_url", serverUrl},
                  {"require_approval", requireApproval}};
        if (headers) j["headers"] = *headers;
        return j;
    }

    static McpTool fromJson(const json& j) {
        McpTool tool;
        tool.serverLabel = j.at("server_label").get<std::string>();
        tool.serverUrl = j.at("server_url").get<std::string>();
        tool.requireApproval = j.value("require_approval", "default");
        if (j.contains("headers")) tool.headers = j["headers"];
        return tool;
    }
};

using ToolVariant = std::variant<FunctionTool, WebSearchTool, FileSearchTool, CodeInterpreterTool,
                                 ImageGenerationTool, McpTool>;

enum class ToolChoiceMode { None, Auto, Required };

inline std::string toString(ToolChoiceMode mode) {
    switch (mode) {
        case ToolChoiceMode::None:
            return "none";
        case ToolChoiceMode::Auto:
            return "auto";
        case ToolChoiceMode::Required:
            return "required";
    }
    return "";
}

inline ToolChoiceMode toolChoiceModeFromString(const std::string& str) {
    if (str == "none") return ToolChoiceMode::None;
    if (str == "auto") return ToolChoiceMode::Auto;
    if (str == "required") return ToolChoiceMode::Required;
    throw std::invalid_argument("Invalid tool choice mode: " + str);
}

// Function call output for tool responses
struct FunctionCallOutput {
    std::string type = "function_call_output";
    std::string callId;
    std::string output;

    json toJson() const { return json{{"type", type}, {"call_id", callId}, {"output", output}}; }

    static FunctionCallOutput fromJson(const json& j) {
        FunctionCallOutput output;
        output.callId = j.at("call_id").get<std::string>();
        output.output = j.at("output").get<std::string>();
        return output;
    }
};

// MCP approval response
struct McpApprovalResponse {
    std::string type = "mcp_approval_response";
    bool approve;
    std::string approvalRequestId;

    json toJson() const {
        return json{
            {"type", type}, {"approve", approve}, {"approval_request_id", approvalRequestId}};
    }

    static McpApprovalResponse fromJson(const json& j) {
        McpApprovalResponse response;
        response.approve = j.at("approve").get<bool>();
        response.approvalRequestId = j.at("approval_request_id").get<std::string>();
        return response;
    }
};

// Response status enumeration
enum class ResponseStatus { Queued, InProgress, Completed, Failed, Cancelled, Incomplete };

inline std::string toString(ResponseStatus status) {
    switch (status) {
        case ResponseStatus::Queued:
            return "queued";
        case ResponseStatus::InProgress:
            return "in_progress";
        case ResponseStatus::Completed:
            return "completed";
        case ResponseStatus::Failed:
            return "failed";
        case ResponseStatus::Cancelled:
            return "cancelled";
        case ResponseStatus::Incomplete:
            return "incomplete";
    }
    return "";
}

inline ResponseStatus responseStatusFromString(const std::string& str) {
    if (str == "queued") return ResponseStatus::Queued;
    if (str == "in_progress") return ResponseStatus::InProgress;
    if (str == "completed") return ResponseStatus::Completed;
    if (str == "failed") return ResponseStatus::Failed;
    if (str == "cancelled") return ResponseStatus::Cancelled;
    if (str == "incomplete") return ResponseStatus::Incomplete;
    throw std::invalid_argument("Invalid response status: " + str);
}

// Responses API request (complete implementation)
struct ResponsesRequest {
    // Required fields
    std::string model;
    std::optional<ResponsesInput> input;

    // Optional configuration (all truly optional - can be omitted entirely)
    std::optional<std::vector<std::string>> include;  // What to include in response
    std::optional<std::string> instructions;          // System-level instructions
    std::optional<int> maxOutputTokens;
    std::optional<int> maxToolCalls;  // Maximum number of tool calls
    std::optional<std::unordered_map<std::string, std::string>> metadata;
    std::optional<bool> parallelToolCalls;
    std::optional<std::string> previousResponseID;  // For conversation continuity
    std::optional<std::string> prompt;              // Alternative to instructions
    std::optional<json> reasoning;                  // Reasoning configuration
    std::optional<std::string> serviceTier;  // "auto", "default", "flex", "scale", "priority"
    std::optional<bool> store;               // Whether to store for retrieval
    std::optional<bool> stream;              // Enable streaming
    std::optional<bool> background;          // Background processing for long tasks
    std::optional<double> temperature;
    std::optional<TextOutputConfig> text;  // Output format configuration
    ToolChoiceMode toolChoice = ToolChoiceMode::Auto;
    std::optional<std::vector<ToolVariant>> tools;
    std::optional<int> topLogprobs;  // Number of top logprobs to return
    std::optional<double> topP;
    std::optional<std::string> truncation;       // "auto" or "disabled"
    std::optional<std::string> user;             // User identifier
    std::optional<std::string> reasoningEffort;  // "low", "medium", "high" for reasoning models

    /**
     * Check if a parameter is supported for the current model
     */
    bool isParameterSupported(const std::string& paramName) const {
        // Convert model string to enum for easier checking
        auto modelEnum = modelFromString(model);

        // Reasoning models (O-series + GPT-5) have different parameter support
        if (modelEnum == Model::GPT_5 || modelEnum == Model::GPT_5_Mini ||
            modelEnum == Model::GPT_5_Nano || modelEnum == Model::O3 ||
            modelEnum == Model::O3_Mini || modelEnum == Model::O1 || modelEnum == Model::O1_Mini ||
            modelEnum == Model::O1_Preview || modelEnum == Model::O1_Pro ||
            modelEnum == Model::O4_Mini || modelEnum == Model::O4_Mini_Deep_Research) {
            // Parameters NOT supported by reasoning models
            if (paramName == "temperature" || paramName == "top_p" || paramName == "top_logprobs" ||
                paramName == "truncation") {
                return false;
            }
        }

        // All other parameters are supported by all models
        return true;
    }

    json toJson() const {
        json j = {{"model", model}, {"tool_choice", toString(toolChoice)}};

        // Only include parameters that are supported by the model
        if (input) j["input"] = input->toJson();
        if (include && isParameterSupported("include")) j["include"] = *include;
        if (instructions && isParameterSupported("instructions")) j["instructions"] = *instructions;
        if (maxOutputTokens && isParameterSupported("max_output_tokens"))
            j["max_output_tokens"] = *maxOutputTokens;
        if (maxToolCalls && isParameterSupported("max_tool_calls"))
            j["max_tool_calls"] = *maxToolCalls;
        if (metadata && isParameterSupported("metadata")) j["metadata"] = *metadata;
        if (parallelToolCalls && isParameterSupported("parallel_tool_calls"))
            j["parallel_tool_calls"] = *parallelToolCalls;
        if (previousResponseID && isParameterSupported("previous_response_id"))
            j["previous_response_id"] = *previousResponseID;
        if (prompt && isParameterSupported("prompt")) j["prompt"] = *prompt;
        if (reasoning && isParameterSupported("reasoning")) j["reasoning"] = *reasoning;
        if (serviceTier && isParameterSupported("service_tier")) j["service_tier"] = *serviceTier;
        if (store && isParameterSupported("store")) j["store"] = *store;
        if (stream && isParameterSupported("stream")) j["stream"] = *stream;
        if (background && isParameterSupported("background")) j["background"] = *background;
        if (temperature && isParameterSupported("temperature")) j["temperature"] = *temperature;
        if (text && isParameterSupported("text")) j["text"] = text->toJson();
        if (topLogprobs && isParameterSupported("top_logprobs")) j["top_logprobs"] = *topLogprobs;
        if (topP && isParameterSupported("top_p")) j["top_p"] = *topP;
        if (truncation && isParameterSupported("truncation")) j["truncation"] = *truncation;
        if (user && isParameterSupported("user")) j["user"] = *user;
        if (reasoningEffort && isParameterSupported("reasoning_effort"))
            j["reasoning_effort"] = *reasoningEffort;
        if (tools && isParameterSupported("tools")) {
            json toolsArray = json::array();
            for (const auto& tool : *tools) {
                std::visit([&toolsArray](const auto& t) { toolsArray.push_back(t.toJson()); },
                           tool);
            }
            j["tools"] = toolsArray;
        }
        return j;
    }

    static ResponsesRequest fromLLMRequest(const LLMRequest& request);
    static ResponsesRequest fromJson(const json& j);
};

// Output items in the response
struct OutputMessage {
    std::string id;
    std::string type = "message";
    std::string role = "assistant";
    std::vector<json> content;  // Content blocks (text, images, etc.)
    std::optional<std::string> status;

    static OutputMessage fromJson(const json& j) {
        OutputMessage msg;
        msg.id = j.at("id").get<std::string>();
        msg.role = j.value("role", "assistant");
        msg.content = j.at("content").get<std::vector<json>>();
        if (j.contains("status")) msg.status = j["status"].get<std::string>();
        return msg;
    }
};

struct FunctionCall {
    std::string id;
    std::string type = "function_call";
    std::string name;
    json arguments;
    std::optional<std::string> status;

    static FunctionCall fromJson(const json& j) {
        FunctionCall call;
        call.id = j.at("id").get<std::string>();
        call.name = j.at("name").get<std::string>();
        call.arguments = j.at("arguments");
        if (j.contains("status")) call.status = j["status"].get<std::string>();
        return call;
    }
};

struct WebSearchCall {
    std::string id;
    std::string type = "web_search_call";
    std::string status;

    static WebSearchCall fromJson(const json& j) {
        WebSearchCall call;
        call.id = j.at("id").get<std::string>();
        call.status = j.at("status").get<std::string>();
        return call;
    }
};

struct ImageGenerationCall {
    std::string id;
    std::string type = "image_generation_call";
    std::string status;
    std::optional<std::string> result;  // Base64 encoded image

    static ImageGenerationCall fromJson(const json& j) {
        ImageGenerationCall call;
        call.id = j.at("id").get<std::string>();
        call.status = j.at("status").get<std::string>();
        if (j.contains("result")) call.result = j["result"].get<std::string>();
        return call;
    }
};

struct McpApprovalRequest {
    std::string id;
    std::string type = "mcp_approval_request";
    std::string name;
    json arguments;
    std::string serverLabel;

    static McpApprovalRequest fromJson(const json& j) {
        McpApprovalRequest request;
        request.id = j.at("id").get<std::string>();
        request.name = j.at("name").get<std::string>();
        request.arguments = j.at("arguments");
        request.serverLabel = j.at("server_label").get<std::string>();
        return request;
    }
};

using OutputItem = std::variant<OutputMessage, FunctionCall, WebSearchCall, ImageGenerationCall,
                                McpApprovalRequest>;

// Complete Responses API response
struct ResponsesResponse {
    std::string id;
    std::string object = "response";
    double createdAt = 0;
    ResponseStatus status = ResponseStatus::Completed;
    std::optional<json> error;
    std::optional<json> incompleteDetails;
    std::optional<std::string> instructions;
    std::optional<int> maxOutputTokens;
    std::string model;
    std::vector<OutputItem> output;
    std::optional<std::string> outputText;  // Convenience field for text output
    bool parallelToolCalls = false;
    std::optional<std::string> previousResponseId;
    std::optional<json> reasoning;
    bool store = true;
    std::optional<json> text;
    std::optional<json> toolChoice;
    std::vector<json> tools;
    std::optional<double> topP;
    std::optional<std::string> truncation;
    LLMUsage usage;
    std::optional<std::string> user;
    std::optional<json> metadata;
    std::optional<std::string> reasoningEffort;

    static ResponsesResponse fromJson(const json& j);
    LLMResponse toLLMResponse(bool expectStructuredOutput = false) const;

    // Convenience methods
    std::string getOutputText() const;
    std::vector<FunctionCall> getFunctionCalls() const;
    std::vector<ImageGenerationCall> getImageGenerations() const;
    bool isCompleted() const { return status == ResponseStatus::Completed; }
    bool hasError() const { return error.has_value() && !error->is_null(); }
};

/**
 * OpenAI Chat Completions API Types (Traditional Conversational API)
 */

struct ChatMessage {
    std::string role;  // "system", "user", "assistant", "tool"
    std::string content;
    std::optional<std::string> name;
    std::optional<json> toolCalls;
    std::optional<std::string> toolCallId;

    json toJson() const {
        json j = {{"role", role}, {"content", content}};
        if (name) j["name"] = *name;
        if (toolCalls) j["tool_calls"] = *toolCalls;
        if (toolCallId) j["tool_call_id"] = *toolCallId;
        return j;
    }

    static ChatMessage fromJson(const json& j) {
        ChatMessage msg;
        msg.role = j.at("role").get<std::string>();
        msg.content = j.at("content").get<std::string>();
        if (j.contains("name")) msg.name = j["name"].get<std::string>();
        if (j.contains("tool_calls")) msg.toolCalls = j["tool_calls"];
        if (j.contains("tool_call_id")) msg.toolCallId = j["tool_call_id"].get<std::string>();
        return msg;
    }
};

struct ChatCompletionRequest {
    std::string model;
    std::vector<ChatMessage> messages;
    std::optional<double> temperature;
    std::optional<int> maxTokens;
    std::optional<double> topP;
    std::optional<int> n;
    std::optional<bool> stream;
    std::optional<std::vector<std::string>> stop;
    std::optional<double> presencePenalty;
    std::optional<double> frequencyPenalty;
    std::optional<json> logitBias;
    std::optional<std::string> user;
    std::optional<json> responseFormat;  // For JSON mode
    std::optional<int> seed;
    std::optional<std::vector<json>> tools;
    std::optional<std::string> toolChoice;

    json toJson() const {
        json j = {{"model", model}, {"messages", json::array()}};

        for (const auto& msg : messages) {
            j["messages"].push_back(msg.toJson());
        }

        if (temperature) j["temperature"] = *temperature;
        if (maxTokens) j["max_tokens"] = *maxTokens;
        if (topP) j["top_p"] = *topP;
        if (n) j["n"] = *n;
        if (stream) j["stream"] = *stream;
        if (stop) j["stop"] = *stop;
        if (presencePenalty) j["presence_penalty"] = *presencePenalty;
        if (frequencyPenalty) j["frequency_penalty"] = *frequencyPenalty;
        if (logitBias) j["logit_bias"] = *logitBias;
        if (user) j["user"] = *user;
        if (responseFormat) j["response_format"] = *responseFormat;
        if (seed) j["seed"] = *seed;
        if (tools) j["tools"] = *tools;
        if (toolChoice) j["tool_choice"] = *toolChoice;

        return j;
    }

    static ChatCompletionRequest fromLLMRequest(const LLMRequest& request);
    LLMRequest toLLMRequest() const;
};

struct ChatCompletionChoice {
    int index;
    ChatMessage message;
    std::optional<std::string> finishReason;
    std::optional<json> logprobs;

    static ChatCompletionChoice fromJson(const json& j) {
        ChatCompletionChoice choice;
        choice.index = j.at("index").get<int>();
        choice.message = ChatMessage::fromJson(j.at("message"));
        if (j.contains("finish_reason") && !j["finish_reason"].is_null()) {
            choice.finishReason = j["finish_reason"].get<std::string>();
        }
        if (j.contains("logprobs")) choice.logprobs = j["logprobs"];
        return choice;
    }
};

struct ChatCompletionResponse {
    std::string id;
    std::string object;
    int64_t created;
    std::string model;
    std::vector<ChatCompletionChoice> choices;
    LLMUsage usage;
    std::optional<std::string> systemFingerprint;

    static ChatCompletionResponse fromJson(const json& j);
    LLMResponse toLLMResponse(bool expectStructuredOutput = false) const;
};

/**
 * OpenAI API Configuration
 */
struct OpenAIConfig {
    std::string apiKey;
    std::string baseUrl = "https://api.openai.com/v1";
    std::string organization;
    std::string project;
    std::string defaultModel = "gpt-4o-mini";  // Default model for convenience
    int timeoutSeconds = 30;
    int maxRetries = 3;
    bool enableDeprecationWarnings = true;

    json toJson() const {
        return json{{"api_key", apiKey},
                    {"base_url", baseUrl},
                    {"organization", organization},
                    {"project", project},
                    {"timeout_seconds", timeoutSeconds},
                    {"max_retries", maxRetries},
                    {"enable_deprecation_warnings", enableDeprecationWarnings}};
    }

    static OpenAIConfig fromJson(const json& j) {
        OpenAIConfig config;
        if (j.contains("api_key")) config.apiKey = j["api_key"].get<std::string>();
        if (j.contains("base_url")) config.baseUrl = j["base_url"].get<std::string>();
        if (j.contains("organization")) config.organization = j["organization"].get<std::string>();
        if (j.contains("project")) config.project = j["project"].get<std::string>();
        if (j.contains("timeout_seconds")) config.timeoutSeconds = j["timeout_seconds"].get<int>();
        if (j.contains("max_retries")) config.maxRetries = j["max_retries"].get<int>();
        if (j.contains("enable_deprecation_warnings")) {
            config.enableDeprecationWarnings = j["enable_deprecation_warnings"].get<bool>();
        }
        return config;
    }
};

/**
 * API Type Detection and Routing
 */
enum class ApiType {
    RESPONSES,         // New structured output API
    CHAT_COMPLETIONS,  // Traditional conversation API
    AUTO_DETECT
};

/**
 * Utility functions
 */
ApiType detectApiType(const LLMRequest& request);
bool supportsResponses(const std::string& model);
bool supportsChatCompletions(const std::string& model);

std::string getRecommendedApiForModel(const std::string& model);

// Model lists for different APIs
const std::vector<std::string> RESPONSES_MODELS = {
    "gpt-5",        "gpt-5-mini",          "gpt-5-nano",  "gpt-4o", "gpt-4o-mini", "gpt-4.1",
    "gpt-4.1-nano", "gpt-4.1-mini",        "gpt-image-1", "o1",     "o3-mini",     "o3",
    "o4-mini",      "computer-use-preview"};

const std::vector<std::string> CHAT_COMPLETION_MODELS = {"gpt-4", "gpt-4-turbo", "gpt-4o",
                                                         "gpt-4o-mini", "gpt-3.5-turbo"};

/**
 * Implementation stubs for conversion methods
 * These will be implemented in the source files
 */

// ResponsesRequest conversion methods
inline ResponsesRequest ResponsesRequest::fromLLMRequest(const LLMRequest& request) {
    ResponsesRequest responsesReq;
    responsesReq.model = request.config.model;

    // Map prompt to OpenAI instructions field
    if (!request.prompt.empty()) {
        responsesReq.instructions = request.prompt;
    }

    // Map context to OpenAI inputValues
    if (!request.context.empty()) {
        // Convert context (vector of json) to InputMessages
        std::vector<InputMessage> messages;

        for (const auto& contextItem : request.context) {
            // Case 1: Single JSON object with role/content
            if (contextItem.is_object() && contextItem.contains("role") &&
                contextItem.contains("content")) {
                InputMessage msg;
                msg.role = InputMessage::stringToRole(contextItem["role"].get<std::string>());
                msg.content = contextItem["content"].get<std::string>();
                messages.push_back(msg);
                continue;
            }

            // Case 2: Array of message-like objects [{role, content}, ...]
            if (contextItem.is_array()) {
                for (const auto& item : contextItem) {
                    if (item.is_object() && item.contains("role") && item.contains("content")) {
                        InputMessage msg;
                        msg.role = InputMessage::stringToRole(item["role"].get<std::string>());
                        msg.content = item["content"].get<std::string>();
                        messages.push_back(msg);
                    }
                }
                continue;
            }

            // Fallback: stringify unknown item as a user message
            InputMessage msg;
            msg.role = InputMessage::Role::User;
            msg.content = contextItem.dump();
            messages.push_back(msg);
        }

        responsesReq.input = ResponsesInput::fromContentList(messages);
    } else if (!request.prompt.empty()) {
        // If context is empty but prompt is present, use prompt as input
        responsesReq.input = ResponsesInput::fromText(request.prompt);
    } else {
        // If both context and prompt are empty, do not set input at all
        responsesReq.input = std::nullopt;
    }
    responsesReq.toolChoice =
        ToolChoiceMode::Auto;  // Explicitly initialize to fix cppcheck warning
    if (request.config.maxTokens.has_value() && *request.config.maxTokens > 0) {
        responsesReq.maxOutputTokens = *request.config.maxTokens;
    }
    // Only set temperature if it's provided and valid
    if (request.config.temperature.has_value() && *request.config.temperature >= 0.0f) {
        responsesReq.temperature = static_cast<double>(*request.config.temperature);
    }
    if (!request.previousResponseId.empty()) {
        responsesReq.previousResponseID = request.previousResponseId;
    }

    // Handle JSON schema for structured outputs
    if (request.config.schemaObject.has_value()) {
        // Use the structured schema object directly
        const json& schemaJson = request.config.schemaObject.value();
        // Use the function name as schema name (like aideas-core does)
        std::string schemaName = request.config.functionName;
        if (schemaName.empty()) {
            schemaName = "response_schema";
        }
        responsesReq.text = TextOutputConfig(schemaName, schemaJson, true);
    } else if (!request.config.jsonSchema.empty()) {
        // Fallback to string schema for backward compatibility
        try {
            json schemaJson = json::parse(request.config.jsonSchema);
            // Use the function name as schema name (like aideas-core does)
            std::string schemaName = request.config.functionName;
            if (schemaName.empty()) {
                schemaName = "response_schema";
            }
            responsesReq.text = TextOutputConfig(schemaName, schemaJson, true);
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid JSON schema: " + std::string(e.what()));
        }
    }

    return responsesReq;
}

inline ResponsesRequest ResponsesRequest::fromJson(const json& j) {
    ResponsesRequest req;
    req.model = j.value("model", "gpt-4o");
    req.input = ResponsesInput::fromJson(j.at("input"));

    if (j.contains("include")) req.include = j["include"].get<std::vector<std::string>>();
    if (j.contains("instructions")) req.instructions = j["instructions"].get<std::string>();
    if (j.contains("max_output_tokens")) req.maxOutputTokens = j["max_output_tokens"].get<int>();
    if (j.contains("max_tool_calls")) req.maxToolCalls = j["max_tool_calls"].get<int>();
    if (j.contains("metadata"))
        req.metadata = j["metadata"].get<std::unordered_map<std::string, std::string>>();
    if (j.contains("parallel_tool_calls"))
        req.parallelToolCalls = j["parallel_tool_calls"].get<bool>();
    if (j.contains("previous_response_id"))
        req.previousResponseID = j["previous_response_id"].get<std::string>();
    if (j.contains("prompt")) req.prompt = j["prompt"].get<std::string>();
    if (j.contains("reasoning")) req.reasoning = j["reasoning"];
    if (j.contains("service_tier")) req.serviceTier = j["service_tier"].get<std::string>();
    if (j.contains("store")) req.store = j["store"].get<bool>();
    if (j.contains("stream")) req.stream = j["stream"].get<bool>();
    if (j.contains("background")) req.background = j["background"].get<bool>();
    if (j.contains("temperature")) req.temperature = j["temperature"].get<double>();
    if (j.contains("text")) req.text = TextOutputConfig::fromJson(j["text"]);
    if (j.contains("tool_choice"))
        req.toolChoice = toolChoiceModeFromString(j["tool_choice"].get<std::string>());
    if (j.contains("top_logprobs")) req.topLogprobs = j["top_logprobs"].get<int>();
    if (j.contains("top_p")) req.topP = j["top_p"].get<double>();
    if (j.contains("truncation")) req.truncation = j["truncation"].get<std::string>();
    if (j.contains("user")) req.user = j["user"].get<std::string>();
    if (j.contains("reasoning_effort"))
        req.reasoningEffort = j["reasoning_effort"].get<std::string>();

    // Parse tools array if present
    if (j.contains("tools") && j["tools"].is_array()) {
        std::vector<ToolVariant> tools;
        for (const auto& toolJson : j["tools"]) {
            std::string toolType = toolJson.at("type").get<std::string>();
            if (toolType == "function") {
                tools.push_back(FunctionTool::fromJson(toolJson));
            } else if (toolType == "web_search") {
                tools.push_back(WebSearchTool::fromJson(toolJson));
            } else if (toolType == "file_search") {
                tools.push_back(FileSearchTool::fromJson(toolJson));
            } else if (toolType == "code_interpreter") {
                tools.push_back(CodeInterpreterTool::fromJson(toolJson));
            } else if (toolType == "image_generation") {
                tools.push_back(ImageGenerationTool::fromJson(toolJson));
            } else if (toolType == "mcp") {
                tools.push_back(McpTool::fromJson(toolJson));
            }
        }
        req.tools = tools;
    }

    return req;
}

// ResponsesResponse conversion methods
inline ResponsesResponse ResponsesResponse::fromJson(const json& j) {
    ResponsesResponse resp;
    resp.id = safeGetRequiredJson<std::string>(j, "id");
    resp.object = safeGetJson(j, "object", std::string("response"));
    resp.createdAt = safeGetJson(j, "created_at", 0.0);
    resp.status = responseStatusFromString(safeGetJson(j, "status", std::string("completed")));
    resp.model = safeGetRequiredJson<std::string>(j, "model");

    // Use utility functions for optional fields
    resp.error = safeGetOptionalJson<json>(j, "error");
    resp.incompleteDetails = safeGetOptionalJson<json>(j, "incomplete_details");
    resp.instructions = safeGetOptionalJson<std::string>(j, "instructions");
    if (j.contains("max_output_tokens"))
        resp.maxOutputTokens = safeGetJson(j, "max_output_tokens", 0);
    resp.outputText = safeGetOptionalJson<std::string>(j, "output_text");
    resp.parallelToolCalls = safeGetJson(j, "parallel_tool_calls", false);
    resp.previousResponseId = safeGetOptionalJson<std::string>(j, "previous_response_id");
    resp.reasoning = safeGetOptionalJson<json>(j, "reasoning");
    resp.store = safeGetJson(j, "store", true);
    resp.text = safeGetOptionalJson<json>(j, "text");
    resp.toolChoice = safeGetOptionalJson<json>(j, "tool_choice");
    resp.tools = safeGetJson(j, "tools", std::vector<json>());
    if (j.contains("top_p")) resp.topP = safeGetJson(j, "top_p", 0.0);
    resp.truncation = safeGetOptionalJson<std::string>(j, "truncation");
    resp.user = safeGetOptionalJson<std::string>(j, "user");
    resp.metadata = safeGetOptionalJson<json>(j, "metadata");
    resp.reasoningEffort = safeGetOptionalJson<std::string>(j, "reasoning_effort");

    // Parse usage
    if (j.contains("usage")) {
        const auto& usage = j["usage"];
        resp.usage.inputTokens = safeGetJson(usage, "input_tokens", 0);
        resp.usage.outputTokens = safeGetJson(usage, "output_tokens", 0);
    }

    // Parse output array
    if (j.contains("output") && j["output"].is_array()) {
        for (const auto& outputJson : j["output"]) {
            std::string outputType = outputJson.at("type").get<std::string>();
            if (outputType == "message") {
                resp.output.push_back(OutputMessage::fromJson(outputJson));
            } else if (outputType == "function_call") {
                resp.output.push_back(FunctionCall::fromJson(outputJson));
            } else if (outputType == "web_search_call") {
                resp.output.push_back(WebSearchCall::fromJson(outputJson));
            } else if (outputType == "image_generation_call") {
                resp.output.push_back(ImageGenerationCall::fromJson(outputJson));
            } else if (outputType == "mcp_approval_request") {
                resp.output.push_back(McpApprovalRequest::fromJson(outputJson));
            }
        }
    }

    return resp;
}

inline LLMResponse ResponsesResponse::toLLMResponse(bool expectStructuredOutput) const {
    LLMResponse llmResp;
    llmResp.success = (status == ResponseStatus::Completed);
    llmResp.responseId = id;
    llmResp.usage = usage;

    if (hasError()) {
        llmResp.errorMessage = error->dump();
    } else {
        // Extract text output from the response
        std::string textOutput = getOutputText();
        if (!textOutput.empty()) {
            if (expectStructuredOutput) {
                // Parse as JSON for structured output
                llmResp.result = json::parse(textOutput);
            } else {
                // Wrap free-form text in text field
                llmResp.result = json{{"text", textOutput}};
            }
        } else {
            llmResp.result = json::object();
        }

        // Add function calls if any
        auto functionCalls = getFunctionCalls();
        if (!functionCalls.empty()) {
            json calls = json::array();
            std::transform(functionCalls.begin(), functionCalls.end(), std::back_inserter(calls),
                           [](const FunctionCall& call) {
                               return json{{"id", call.id},
                                           {"name", call.name},
                                           {"arguments", call.arguments}};
                           });
            llmResp.result["function_calls"] = calls;
        }

        // Add images if any
        auto images = getImageGenerations();
        if (!images.empty()) {
            json imageArray = json::array();
            for (const auto& img : images) {
                if (img.result) {
                    imageArray.push_back(*img.result);
                }
            }
            llmResp.result["images"] = imageArray;
        }
    }

    return llmResp;
}

// Convenience methods for ResponsesResponse
inline std::string ResponsesResponse::getOutputText() const {
    if (outputText) return *outputText;

    // Extract from output messages
    for (const auto& item : output) {
        if (std::holds_alternative<OutputMessage>(item)) {
            const auto& msg = std::get<OutputMessage>(item);
            auto it = std::find_if(msg.content.begin(), msg.content.end(), [](const json& content) {
                return content.contains("text") && content["type"] == "output_text";
            });
            if (it != msg.content.end()) {
                return (*it)["text"].get<std::string>();
            }
        }
    }
    return "";
}

inline std::vector<FunctionCall> ResponsesResponse::getFunctionCalls() const {
    std::vector<FunctionCall> calls;
    for (const auto& item : output) {
        if (std::holds_alternative<FunctionCall>(item)) {
            calls.push_back(std::get<FunctionCall>(item));
        }
    }
    return calls;
}

inline std::vector<ImageGenerationCall> ResponsesResponse::getImageGenerations() const {
    std::vector<ImageGenerationCall> images;
    for (const auto& item : output) {
        if (std::holds_alternative<ImageGenerationCall>(item)) {
            images.push_back(std::get<ImageGenerationCall>(item));
        }
    }
    return images;
}

// ChatCompletionRequest conversion methods
inline ChatCompletionRequest ChatCompletionRequest::fromLLMRequest(const LLMRequest& request) {
    ChatCompletionRequest chatReq;
    chatReq.model = request.config.model;

    // Convert prompt to messages
    if (!request.prompt.empty()) {
        ChatMessage userMsg;
        userMsg.role = "user";
        userMsg.content = request.prompt;
        chatReq.messages.push_back(userMsg);
    }

    if (request.config.maxTokens.has_value() && *request.config.maxTokens > 0) {
        chatReq.maxTokens = *request.config.maxTokens;
    }
    // Only set temperature if it's provided and valid
    if (request.config.temperature.has_value() && *request.config.temperature >= 0.0f) {
        chatReq.temperature = static_cast<double>(*request.config.temperature);
    }

    return chatReq;
}

inline LLMRequest ChatCompletionRequest::toLLMRequest() const {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = model;
    if (temperature) config.temperature = static_cast<float>(*temperature);
    if (maxTokens) config.maxTokens = *maxTokens;

    std::string prompt;
    if (!messages.empty()) {
        // Use the last user message as prompt
        for (auto it = messages.rbegin(); it != messages.rend(); ++it) {
            if (it->role == "user") {
                prompt = it->content;
                break;
            }
        }
    }

    return LLMRequest(config, prompt);
}

// ChatCompletionResponse conversion methods
inline ChatCompletionResponse ChatCompletionResponse::fromJson(const json& j) {
    ChatCompletionResponse resp;
    resp.id = j.at("id").get<std::string>();
    resp.object = j.at("object").get<std::string>();
    resp.created = j.at("created").get<int64_t>();
    resp.model = j.at("model").get<std::string>();

    if (j.contains("system_fingerprint")) {
        resp.systemFingerprint = j["system_fingerprint"].get<std::string>();
    }

    // Parse choices
    for (const auto& choiceJson : j.at("choices")) {
        resp.choices.push_back(ChatCompletionChoice::fromJson(choiceJson));
    }

    // Parse usage
    if (j.contains("usage")) {
        const auto& usage = j["usage"];
        if (usage.contains("prompt_tokens"))
            resp.usage.inputTokens = usage["prompt_tokens"].get<int>();
        if (usage.contains("completion_tokens"))
            resp.usage.outputTokens = usage["completion_tokens"].get<int>();
    }

    return resp;
}

inline LLMResponse ChatCompletionResponse::toLLMResponse(bool expectStructuredOutput) const {
    LLMResponse llmResp;
    llmResp.success = !choices.empty();
    llmResp.responseId = id;
    llmResp.usage = usage;

    if (!choices.empty()) {
        llmResp.result = json::object();
        llmResp.result["text"] = choices[0].message.content;
        if (choices[0].message.toolCalls) {
            llmResp.result["tool_calls"] = *choices[0].message.toolCalls;
        }
    } else {
        llmResp.errorMessage = "No choices returned";
    }

    return llmResp;
}

/**
 * Utility functions implementation
 */
inline ApiType detectApiType(const LLMRequest& request) {
    const std::string& model = request.config.model;

    // Check if it's a Responses API model
    if (std::any_of(
            RESPONSES_MODELS.begin(), RESPONSES_MODELS.end(),
            [&model](const std::string& responsesModel) { return model == responsesModel; })) {
        return ApiType::RESPONSES;
    }

    // Default to Chat Completions for most models
    return ApiType::CHAT_COMPLETIONS;
}

inline bool supportsResponses(const std::string& model) {
    return std::any_of(
        RESPONSES_MODELS.begin(), RESPONSES_MODELS.end(),
        [&model](const std::string& responsesModel) { return model == responsesModel; });
}

inline bool supportsChatCompletions(const std::string& model) {
    return std::any_of(CHAT_COMPLETION_MODELS.begin(), CHAT_COMPLETION_MODELS.end(),
                       [&model](const std::string& chatModel) { return model == chatModel; });
}

inline std::string getRecommendedApiForModel(const std::string& model) {
    if (supportsResponses(model)) {
        return "Responses API (Recommended)";
    } else if (supportsChatCompletions(model)) {
        return "Chat Completions API";
    }
    return "Unknown";
}

}  // namespace OpenAI
