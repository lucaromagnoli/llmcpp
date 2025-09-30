#pragma once
#include <algorithm>
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/LLMTypes.h"
#include "utils/JsonUtils.h"

using json = nlohmann::json;

namespace OpenAI {

// Forward declarations
struct FunctionTool;
struct WebSearchTool;
struct FileSearchTool;
struct CodeInterpreterTool;
struct ImageGenerationTool;
struct McpTool;

// Output types - full definitions needed for use in vectors
struct FunctionCall {
    std::string id;
    std::string name;
    json arguments;
};

struct ImageGenerationCall {
    std::string url;
    std::string prompt;
    std::optional<json> result;
};

struct OutputMessage {
    std::string role;
    std::string content;
};

struct WebSearchCall {
    std::string query;
    json results;
};

struct McpApprovalRequest {
    std::string toolName;
    json parameters;
    std::string requestId;
};

// Enums and types that need to be defined early
enum class ToolChoiceMode { None, Auto, Required };
enum class ResponseStatus { Queued, InProgress, Completed, Failed, Cancelled, Incomplete };
enum class ApiType {
    RESPONSES,         // New structured output API
    CHAT_COMPLETIONS,  // Traditional conversation API
    AUTO_DETECT
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
                          {"strict", isStrict},
                          {"schema", formatSchema}};

        return json{{"format", formatObj}};
    }

    static TextOutputConfig fromJson(const json& j) {
        if (!j.contains("format")) {
            return TextOutputConfig{};
        }

        const auto& format = j["format"];
        if (!format.contains("name") || !format.contains("json_schema")) {
            return TextOutputConfig{};
        }

        bool strict = true;
        if (format.contains("json_schema") &&
            format["json_schema"].contains("additionalProperties")) {
            strict = !format["json_schema"]["additionalProperties"].get<bool>();
        }

        return TextOutputConfig{format["name"].get<std::string>(), format["json_schema"], strict};
    }

   private:
    std::string formatName;
    json formatSchema;
    bool isStrict = true;
};

// OpenAI-specific tool variant
using ToolVariant = std::variant<FunctionTool, WebSearchTool, FileSearchTool, CodeInterpreterTool,
                                 ImageGenerationTool, McpTool>;

// Helper functions to work with tools in the generic LLMRequestConfig
// Declarations - implementations are in OpenAITypes.cpp after tool definitions
void setTools(LLMRequestConfig& config, const std::vector<ToolVariant>& tools);
bool hasTools(const LLMRequestConfig& config);
json getToolsJson(const LLMRequestConfig& config);

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

// Responses API request structure
struct ResponsesRequest {
    std::string model;
    std::optional<ResponsesInput> input;
    std::vector<std::string> include;
    std::string instructions;
    std::optional<int> maxOutputTokens;
    std::optional<TextOutputConfig> text;
    ToolChoiceMode toolChoice = ToolChoiceMode::Auto;
    std::vector<ToolVariant> tools;
    std::optional<float> topP;
    std::optional<float> temperature;
    std::string user;
    bool store = true;
    std::string reasoningEffort = "medium";
    std::string metadata;
    std::optional<json> reasoning;

    static ResponsesRequest fromLLMRequest(const struct LLMRequest& request);
    static ResponsesRequest fromJson(const json& j);
    json toJson() const;
};

// Responses API response structure
struct ResponsesResponse {
    std::string id;
    std::string object = "response";
    double createdAt = 0.0;
    ResponseStatus status = ResponseStatus::Completed;
    std::string model;
    std::optional<json> error;
    std::optional<json> incompleteDetails;
    std::optional<std::string> instructions;
    int maxOutputTokens = 0;
    std::optional<std::string> outputText;
    bool parallelToolCalls = false;
    std::optional<std::string> previousResponseId;
    std::optional<json> reasoning;
    bool store = true;
    std::optional<json> text;
    std::optional<json> toolChoice;
    std::vector<json> tools;
    std::optional<float> topP;
    std::optional<std::string> truncation;
    std::optional<std::string> user;
    std::optional<json> metadata;
    std::optional<std::string> reasoningEffort;
    LLMUsage usage;
    std::vector<json> output;

    LLMResponse toLLMResponse(bool expectStructuredOutput = false) const;
    std::string getOutputText() const;
    std::vector<FunctionCall> getFunctionCalls() const;
    std::vector<ImageGenerationCall> getImageGenerations() const;
    bool hasError() const;
    bool isCompleted() const { return status == ResponseStatus::Completed; }
    static ResponsesResponse fromJson(const json& j);
};

// OpenAI configuration structure
struct OpenAIConfig {
    std::string apiKey;
    std::string baseUrl = "https://api.openai.com/v1";
    std::string organization;
    std::string project;
    std::map<std::string, std::string> headers;
    int timeoutSeconds = 30;
    int maxRetries = 3;
    bool verifySSL = true;
    bool enableDeprecationWarnings = true;

    json toJson() const {
        json j = {{"api_key", apiKey},
                  {"base_url", baseUrl},
                  {"timeout_seconds", timeoutSeconds},
                  {"max_retries", maxRetries},
                  {"enable_deprecation_warnings", enableDeprecationWarnings}};
        if (!organization.empty()) j["organization"] = organization;
        if (!project.empty()) j["project"] = project;
        return j;
    }

    static OpenAIConfig fromJson(const json& j) {
        OpenAIConfig config;
        if (j.contains("api_key")) config.apiKey = j["api_key"].get<std::string>();
        if (j.contains("base_url")) config.baseUrl = j["base_url"].get<std::string>();
        if (j.contains("organization")) config.organization = j["organization"].get<std::string>();
        if (j.contains("project")) config.project = j["project"].get<std::string>();
        if (j.contains("timeout_seconds")) config.timeoutSeconds = j["timeout_seconds"].get<int>();
        if (j.contains("max_retries")) config.maxRetries = j["max_retries"].get<int>();
        if (j.contains("enable_deprecation_warnings"))
            config.enableDeprecationWarnings = j["enable_deprecation_warnings"].get<bool>();
        return config;
    }
};

// TextOutputConfig already defined at top of namespace

// Chat message structure for Chat Completion API
struct ChatMessage {
    std::string role;
    std::string content;
    std::string name;

    json toJson() const {
        json j = {{"role", role}, {"content", content}};
        if (!name.empty()) {
            j["name"] = name;
        }
        return j;
    }

    static ChatMessage fromJson(const json& j) {
        ChatMessage msg;
        msg.role = j.at("role").get<std::string>();
        msg.content = j.at("content").get<std::string>();
        if (j.contains("name")) {
            msg.name = j["name"].get<std::string>();
        }
        return msg;
    }
};

// Chat Completion API structures
struct ChatCompletionRequest {
    std::string model;
    std::vector<ChatMessage> messages;
    std::optional<float> temperature;
    std::optional<int> maxTokens;
    std::optional<std::vector<std::string>> stop;
    std::optional<bool> stream;
    std::optional<std::string> user;
    std::optional<std::vector<ToolVariant>> tools;
    std::optional<ToolChoiceMode> toolChoice;
    std::optional<TextOutputConfig> text;

    json toJson() const {
        json j = {{"model", model}};
        json msgs = json::array();
        for (const auto& msg : messages) {
            msgs.push_back(msg.toJson());
        }
        j["messages"] = msgs;
        if (temperature.has_value()) j["temperature"] = temperature.value();
        if (maxTokens.has_value()) j["max_tokens"] = maxTokens.value();
        if (stop.has_value()) j["stop"] = stop.value();
        if (stream.has_value()) j["stream"] = stream.value();
        if (user.has_value()) j["user"] = user.value();
        return j;
    }

    static ChatCompletionRequest fromLLMRequest(const struct LLMRequest& request);
    struct LLMRequest toLLMRequest() const;
    static ChatCompletionRequest fromJson(const json& j);
};

struct ChatCompletionChoice {
    int index;
    Message message;
    std::optional<std::string> finishReason;
    std::optional<json> logprobs;

    static ChatCompletionChoice fromJson(const json& j);
};

struct ChatCompletionResponse {
    std::string id;
    std::string object;
    int64_t created;
    std::string model;
    std::vector<ChatCompletionChoice> choices;
    std::optional<std::string> systemFingerprint;
    LLMUsage usage;

    static ChatCompletionResponse fromJson(const json& j);
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

// Define ToolVariant after all tool structs are complete
using ToolVariant = std::variant<FunctionTool, WebSearchTool, FileSearchTool, CodeInterpreterTool,
                                 ImageGenerationTool, McpTool>;

// ToolChoiceMode enum moved to top of namespace

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

// ResponseStatus enum moved to top of namespace

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

// Forward declaration for detectApiType
ApiType detectApiType(const LLMRequest& request);

inline ResponseStatus responseStatusFromString(const std::string& str) {
    if (str == "queued") return ResponseStatus::Queued;
    if (str == "in_progress") return ResponseStatus::InProgress;
    if (str == "completed") return ResponseStatus::Completed;
    if (str == "failed") return ResponseStatus::Failed;
    if (str == "cancelled") return ResponseStatus::Cancelled;
    if (str == "incomplete") return ResponseStatus::Incomplete;
    throw std::invalid_argument("Invalid response status: " + str);
}

// Close the OpenAI namespace

}  // namespace OpenAI

// Make OpenAI types globally accessible
using OpenAI::FunctionCallOutput;
using OpenAI::McpApprovalResponse;
using OpenAI::OpenAIConfig;
using OpenAI::ResponsesRequest;
using OpenAI::ResponsesResponse;
using OpenAI::ResponseStatus;
using OpenAI::ToolVariant;

// All structs moved to OpenAI namespace

/**
 * API Type Detection and Routing
 */
// ApiType enum moved to top of namespace

/**
 * Utility functions
 */
OpenAI::ApiType detectApiType(const LLMRequest& request);
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

// Conversion methods - implementations moved to .cpp file to avoid circular dependency

// Note: fromJson implementations are in the .cpp file to avoid circular dependencies

// ResponsesResponse conversion methods - implementation in .cpp file

// Implementation moved to .cpp file

// Convenience methods for ResponsesResponse - implementation in .cpp file

// ChatCompletionRequest conversion methods - implementation in .cpp file

// ChatCompletionResponse conversion methods - implementation in .cpp file

/**
 * Utility functions implementation
 */
// Implementation moved to .cpp file to avoid circular dependency

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
