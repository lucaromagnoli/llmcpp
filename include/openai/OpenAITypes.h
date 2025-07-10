#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "core/LLMTypes.h"

using json = nlohmann::json;

namespace OpenAI {

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
            for (const auto& message : contentList) {
                messageArray.push_back(message.toJson());
            }
            return messageArray;
        }
    }

    static ResponsesInput fromJson(const json& j) {
        if (j.is_string()) {
            return fromText(j.get<std::string>());
        } else if (j.is_array()) {
            std::vector<InputMessage> messages;
            for (const auto& item : j) {
                messages.push_back(InputMessage::fromJson(item));
            }
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
enum class ResponseStatus { Queued, InProgress, Completed, Failed, Cancelled };

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
    }
    return "";
}

inline ResponseStatus responseStatusFromString(const std::string& str) {
    if (str == "queued") return ResponseStatus::Queued;
    if (str == "in_progress") return ResponseStatus::InProgress;
    if (str == "completed") return ResponseStatus::Completed;
    if (str == "failed") return ResponseStatus::Failed;
    if (str == "cancelled") return ResponseStatus::Cancelled;
    throw std::invalid_argument("Invalid response status: " + str);
}

// Responses API request (complete implementation)
struct ResponsesRequest {
    // Required fields
    ResponsesInput input;
    std::string model = "gpt-4o";

    // Optional configuration
    std::optional<std::vector<std::string>> include;  // What to include in response
    std::optional<std::string> instructions;          // System-level instructions
    std::optional<int> maxOutputTokens;
    std::optional<std::unordered_map<std::string, std::string>> metadata;
    std::optional<bool> parallelToolCalls;
    std::optional<std::string> previousResponseID;  // For conversation continuity
    std::optional<bool> store;                      // Whether to store for retrieval
    std::optional<bool> stream;                     // Enable streaming
    std::optional<bool> background;                 // Background processing for long tasks
    std::optional<double> temperature;
    std::optional<TextOutputConfig> text;  // Output format configuration
    ToolChoiceMode toolChoice = ToolChoiceMode::Auto;
    std::optional<std::vector<ToolVariant>> tools;
    std::optional<double> topP;
    std::optional<std::string> truncation;       // "auto" or "disabled"
    std::optional<std::string> user;             // User identifier
    std::optional<std::string> reasoningEffort;  // "low", "medium", "high" for reasoning models

    json toJson() const {
        json j = {
            {"model", model}, {"input", input.toJson()}, {"tool_choice", toString(toolChoice)}};

        if (include) j["include"] = *include;
        if (instructions) j["instructions"] = *instructions;
        if (maxOutputTokens) j["max_output_tokens"] = *maxOutputTokens;
        if (metadata) j["metadata"] = *metadata;
        if (parallelToolCalls) j["parallel_tool_calls"] = *parallelToolCalls;
        if (previousResponseID) j["previous_response_id"] = *previousResponseID;
        if (store) j["store"] = *store;
        if (stream) j["stream"] = *stream;
        if (background) j["background"] = *background;
        if (temperature) j["temperature"] = *temperature;
        if (text) j["text"] = text->toJson();
        if (topP) j["top_p"] = *topP;
        if (truncation) j["truncation"] = *truncation;
        if (user) j["user"] = *user;
        if (reasoningEffort) j["reasoning_effort"] = *reasoningEffort;

        if (tools) {
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
    LLMResponse toLLMResponse() const;

    // Convenience methods
    std::string getOutputText() const;
    std::vector<FunctionCall> getFunctionCalls() const;
    std::vector<ImageGenerationCall> getImageGenerations() const;
    bool isCompleted() const { return status == ResponseStatus::Completed; }
    bool hasError() const { return error.has_value(); }
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
    LLMResponse toLLMResponse() const;
};

/**
 * OpenAI API Configuration
 */
struct OpenAIConfig {
    std::string apiKey;
    std::string baseUrl = "https://api.openai.com/v1";
    std::string organization;
    std::string project;
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
    "gpt-4o", "gpt-4o-mini", "gpt-4.1", "gpt-4.1-nano", "gpt-4.1-mini",        "gpt-image-1",
    "o1",     "o3-mini",     "o3",      "o4-mini",      "computer-use-preview"};

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
    responsesReq.input = ResponsesInput::fromText(request.prompt);
    if (request.config.maxTokens > 0) {
        responsesReq.maxOutputTokens = request.config.maxTokens;
    }
    if (request.config.randomness >= 0.0f) {
        responsesReq.temperature = static_cast<double>(request.config.randomness);
    }
    if (!request.previousResponseId.empty()) {
        responsesReq.previousResponseID = request.previousResponseId;
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
    if (j.contains("metadata"))
        req.metadata = j["metadata"].get<std::unordered_map<std::string, std::string>>();
    if (j.contains("parallel_tool_calls"))
        req.parallelToolCalls = j["parallel_tool_calls"].get<bool>();
    if (j.contains("previous_response_id"))
        req.previousResponseID = j["previous_response_id"].get<std::string>();
    if (j.contains("store")) req.store = j["store"].get<bool>();
    if (j.contains("stream")) req.stream = j["stream"].get<bool>();
    if (j.contains("background")) req.background = j["background"].get<bool>();
    if (j.contains("temperature")) req.temperature = j["temperature"].get<double>();
    if (j.contains("text")) req.text = TextOutputConfig::fromJson(j["text"]);
    if (j.contains("tool_choice"))
        req.toolChoice = toolChoiceModeFromString(j["tool_choice"].get<std::string>());
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
    resp.id = j.at("id").get<std::string>();
    resp.object = j.value("object", "response");
    resp.createdAt = j.value("created_at", 0.0);
    resp.status = responseStatusFromString(j.value("status", "completed"));
    resp.model = j.at("model").get<std::string>();

    if (j.contains("error")) resp.error = j["error"];
    if (j.contains("incomplete_details")) resp.incompleteDetails = j["incomplete_details"];
    if (j.contains("instructions")) resp.instructions = j["instructions"].get<std::string>();
    if (j.contains("max_output_tokens")) resp.maxOutputTokens = j["max_output_tokens"].get<int>();
    if (j.contains("output_text")) resp.outputText = j["output_text"].get<std::string>();
    if (j.contains("parallel_tool_calls"))
        resp.parallelToolCalls = j["parallel_tool_calls"].get<bool>();
    if (j.contains("previous_response_id"))
        resp.previousResponseId = j["previous_response_id"].get<std::string>();
    if (j.contains("reasoning")) resp.reasoning = j["reasoning"];
    if (j.contains("store")) resp.store = j["store"].get<bool>();
    if (j.contains("text")) resp.text = j["text"];
    if (j.contains("tool_choice")) resp.toolChoice = j["tool_choice"];
    if (j.contains("tools")) resp.tools = j["tools"].get<std::vector<json>>();
    if (j.contains("top_p")) resp.topP = j["top_p"].get<double>();
    if (j.contains("truncation")) resp.truncation = j["truncation"].get<std::string>();
    if (j.contains("user")) resp.user = j["user"].get<std::string>();
    if (j.contains("metadata")) resp.metadata = j["metadata"];
    if (j.contains("reasoning_effort"))
        resp.reasoningEffort = j["reasoning_effort"].get<std::string>();

    // Parse usage
    if (j.contains("usage")) {
        const auto& usage = j["usage"];
        if (usage.contains("input_tokens"))
            resp.usage.inputTokens = usage["input_tokens"].get<int>();
        if (usage.contains("output_tokens"))
            resp.usage.outputTokens = usage["output_tokens"].get<int>();
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

inline LLMResponse ResponsesResponse::toLLMResponse() const {
    LLMResponse llmResp;
    llmResp.success = (status == ResponseStatus::Completed);
    llmResp.responseId = id;
    llmResp.usage = usage;

    if (hasError()) {
        llmResp.errorMessage = error->dump();
    } else {
        // Extract text output from the response
        llmResp.result = json::object();
        std::string textOutput = getOutputText();
        if (!textOutput.empty()) {
            llmResp.result["text"] = textOutput;
        }

        // Add function calls if any
        auto functionCalls = getFunctionCalls();
        if (!functionCalls.empty()) {
            json calls = json::array();
            for (const auto& call : functionCalls) {
                calls.push_back(
                    {{"id", call.id}, {"name", call.name}, {"arguments", call.arguments}});
            }
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
            for (const auto& content : msg.content) {
                if (content.contains("text") && content["type"] == "output_text") {
                    return content["text"].get<std::string>();
                }
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

    if (request.config.maxTokens > 0) {
        chatReq.maxTokens = request.config.maxTokens;
    }
    if (request.config.randomness >= 0.0f) {
        chatReq.temperature = static_cast<double>(request.config.randomness);
    }

    return chatReq;
}

inline LLMRequest ChatCompletionRequest::toLLMRequest() const {
    LLMRequestConfig config;
    config.client = "openai";
    config.model = model;
    if (temperature) config.randomness = static_cast<float>(*temperature);
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

inline LLMResponse ChatCompletionResponse::toLLMResponse() const {
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
    for (const auto& responsesModel : RESPONSES_MODELS) {
        if (model == responsesModel) {
            return ApiType::RESPONSES;
        }
    }

    // Default to Chat Completions for most models
    return ApiType::CHAT_COMPLETIONS;
}

inline bool supportsResponses(const std::string& model) {
    for (const auto& responsesModel : RESPONSES_MODELS) {
        if (model == responsesModel) {
            return true;
        }
    }
    return false;
}

inline bool supportsChatCompletions(const std::string& model) {
    for (const auto& chatModel : CHAT_COMPLETION_MODELS) {
        if (model == chatModel) {
            return true;
        }
    }
    return false;
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