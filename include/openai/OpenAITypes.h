#pragma once
#include "core/LLMTypes.h"
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>

namespace OpenAI {

/**
 * OpenAI Responses API Types (Modern Structured Output API)
 */

// Text input for the OpenAI API
struct TextInput {
    std::string text;
    std::string type = "input_text";
    
    json toJson() const {
        return json{{"text", text}, {"type", type}};
    }
};

// Image input for the OpenAI API
struct ImageInput {
    std::string detail = "auto";
    std::string type = "input_image";
    std::optional<std::string> fileId;
    std::optional<std::string> imageUrl;
    
    json toJson() const {
        json j = {{"detail", detail}, {"type", type}};
        if (fileId) j["file_id"] = *fileId;
        if (imageUrl) j["image_url"] = *imageUrl;
        return j;
    }
};

// File input for the OpenAI API
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
};

using InputContent = std::variant<TextInput, FileInput, ImageInput>;

// Message input with role
struct InputMessage {
    enum class Role { User, Assistant, System, Developer };
    
    std::variant<std::string, std::vector<InputContent>> content;
    Role role = Role::User;
    std::optional<std::string> type;
    
    static std::string roleToString(Role role) {
        switch (role) {
            case Role::User: return "user";
            case Role::Assistant: return "assistant";
            case Role::System: return "system";
            case Role::Developer: return "developer";
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
                std::visit([&contentArray](const auto& input) {
                    contentArray.push_back(input.toJson());
                }, item);
            }
            j["content"] = contentArray;
        }
        j["role"] = roleToString(role);
        if (type) j["type"] = *type;
        return j;
    }
};

// Responses input (string or message array)
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
};

// Text output configuration for structured responses
class TextOutputConfig {
public:
    TextOutputConfig() = default;
    TextOutputConfig(std::string name, json schema, bool strict = true)
        : formatName(std::move(name)), formatSchema(std::move(schema)), isStrict(strict) {}
    
    json toJson() const {
        json formatObj = {
            {"type", "json_schema"},
            {"name", formatName},
            {"schema", formatSchema},
            {"strict", isStrict}
        };
        return json{{"format", formatObj}};
    }
    
private:
    std::string formatName;
    json formatSchema;
    bool isStrict = true;
};

// Function tool for Responses API
struct FunctionTool {
    std::string name;
    json parameters;
    bool strict = true;
    std::string type = "function";
    std::optional<std::string> description;
    
    json toJson() const {
        json j = {
            {"name", name},
            {"parameters", parameters},
            {"strict", strict},
            {"type", type}
        };
        if (description) j["description"] = *description;
        return j;
    }
};

// Web search tool
struct WebSearchTool {
    std::string type = "web_search";
    std::optional<std::string> searchContextSize;
    
    json toJson() const {
        json j = {{"type", type}};
        if (searchContextSize) j["search_context_size"] = *searchContextSize;
        return j;
    }
};

// File search tool
struct FileSearchTool {
    std::string type = "file_search";
    std::vector<std::string> vectorStoreIds;
    std::optional<json> filters;
    std::optional<int> maxNumResults;
    
    json toJson() const {
        json j = {{"type", type}, {"vector_store_ids", vectorStoreIds}};
        if (filters) j["filters"] = *filters;
        if (maxNumResults) j["max_num_results"] = *maxNumResults;
        return j;
    }
};

using ToolVariant = std::variant<FunctionTool, WebSearchTool, FileSearchTool>;

enum class ToolChoiceMode { None, Auto, Required };

inline std::string toString(ToolChoiceMode mode) {
    switch (mode) {
        case ToolChoiceMode::None: return "none";
        case ToolChoiceMode::Auto: return "auto";
        case ToolChoiceMode::Required: return "required";
    }
    return "";
}

// Responses API request
struct ResponsesRequest {
    ResponsesInput input;
    std::string model = "gpt-4o";
    std::optional<std::vector<std::string>> include;
    std::string instructions;
    std::optional<int> maxOutputTokens;
    std::optional<std::unordered_map<std::string, std::string>> metadata;
    std::optional<bool> parallelToolCalls;
    std::optional<std::string> previousResponseID;
    std::optional<bool> store;
    std::optional<bool> stream;
    std::optional<double> temperature;
    TextOutputConfig text;
    ToolChoiceMode toolChoice = ToolChoiceMode::Auto;
    std::optional<std::vector<ToolVariant>> tools;
    std::optional<double> topP;
    std::optional<std::string> truncation;
    std::optional<std::string> user;
    
    json toJson() const {
        json j = {
            {"model", model},
            {"input", input.toJson()},
            {"instructions", instructions},
            {"text", text.toJson()},
            {"tool_choice", toString(toolChoice)}
        };
        
        if (include) j["include"] = *include;
        if (maxOutputTokens) j["max_output_tokens"] = *maxOutputTokens;
        if (metadata) j["metadata"] = *metadata;
        if (parallelToolCalls) j["parallel_tool_calls"] = *parallelToolCalls;
        if (previousResponseID) j["previous_response_id"] = *previousResponseID;
        if (store) j["store"] = *store;
        if (stream) j["stream"] = *stream;
        if (temperature) j["temperature"] = *temperature;
        if (topP) j["top_p"] = *topP;
        if (truncation) j["truncation"] = *truncation;
        if (user) j["user"] = *user;
        
        if (tools) {
            json toolsArray = json::array();
            for (const auto& tool : *tools) {
                std::visit([&toolsArray](const auto& t) {
                    toolsArray.push_back(t.toJson());
                }, tool);
            }
            j["tools"] = toolsArray;
        }
        
        return j;
    }
    
    // Convert from generic LLMRequest
    static ResponsesRequest fromLLMRequest(const LLMRequest& request);
};

// Responses API response
struct ResponsesResponse {
    std::string id;
    std::string object;
    int64_t createdAt = 0;
    std::string status;
    std::optional<json> error;
    std::optional<json> incompleteDetails;
    std::string instructions;
    std::optional<int> maxOutputTokens;
    std::string model;
    std::vector<json> output;
    bool parallelToolCalls = false;
    std::string previousResponseId;
    std::optional<json> reasoning;
    bool store = false;
    json text;
    json toolChoice;
    std::vector<json> tools;
    std::optional<double> topP;
    std::string truncation;
    LLMUsage usage;
    std::string user;
    std::optional<json> metadata;
    
    static ResponsesResponse fromJson(const json& j);
    LLMResponse toLLMResponse() const;
};

/**
 * OpenAI Chat Completions API Types (Traditional Conversational API)
 */

struct ChatMessage {
    std::string role;     // "system", "user", "assistant", "tool"
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
 * OpenAI Completions API Types (Legacy API - Deprecated)
 */

struct CompletionRequest {
    std::string model;
    std::string prompt;
    std::optional<int> maxTokens;
    std::optional<double> temperature;
    std::optional<double> topP;
    std::optional<int> n;
    std::optional<bool> stream;
    std::optional<int> logprobs;
    std::optional<bool> echo;
    std::optional<std::vector<std::string>> stop;
    std::optional<double> presencePenalty;
    std::optional<double> frequencyPenalty;
    std::optional<int> bestOf;
    std::optional<json> logitBias;
    std::optional<std::string> user;
    std::optional<std::string> suffix;
    
    json toJson() const {
        json j = {{"model", model}, {"prompt", prompt}};
        
        if (maxTokens) j["max_tokens"] = *maxTokens;
        if (temperature) j["temperature"] = *temperature;
        if (topP) j["top_p"] = *topP;
        if (n) j["n"] = *n;
        if (stream) j["stream"] = *stream;
        if (logprobs) j["logprobs"] = *logprobs;
        if (echo) j["echo"] = *echo;
        if (stop) j["stop"] = *stop;
        if (presencePenalty) j["presence_penalty"] = *presencePenalty;
        if (frequencyPenalty) j["frequency_penalty"] = *frequencyPenalty;
        if (bestOf) j["best_of"] = *bestOf;
        if (logitBias) j["logit_bias"] = *logitBias;
        if (user) j["user"] = *user;
        if (suffix) j["suffix"] = *suffix;
        
        return j;
    }
    
    static CompletionRequest fromLLMRequest(const LLMRequest& request);
    LLMRequest toLLMRequest() const;
};

struct CompletionChoice {
    std::string text;
    int index;
    std::optional<json> logprobs;
    std::optional<std::string> finishReason;
    
    static CompletionChoice fromJson(const json& j) {
        CompletionChoice choice;
        choice.text = j.at("text").get<std::string>();
        choice.index = j.at("index").get<int>();
        if (j.contains("logprobs")) choice.logprobs = j["logprobs"];
        if (j.contains("finish_reason") && !j["finish_reason"].is_null()) {
            choice.finishReason = j["finish_reason"].get<std::string>();
        }
        return choice;
    }
};

struct CompletionResponse {
    std::string id;
    std::string object;
    int64_t created;
    std::string model;
    std::vector<CompletionChoice> choices;
    LLMUsage usage;
    
    static CompletionResponse fromJson(const json& j);
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
        return json{
            {"api_key", apiKey},
            {"base_url", baseUrl},
            {"organization", organization},
            {"project", project},
            {"timeout_seconds", timeoutSeconds},
            {"max_retries", maxRetries},
            {"enable_deprecation_warnings", enableDeprecationWarnings}
        };
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
    RESPONSES,           // New structured output API
    CHAT_COMPLETIONS,    // Traditional conversation API
    COMPLETIONS,         // Legacy text completion (deprecated)
    AUTO_DETECT
};

/**
 * Utility functions
 */
ApiType detectApiType(const LLMRequest& request);
bool isLegacyModel(const std::string& model);
bool supportsResponses(const std::string& model);
bool supportsChatCompletions(const std::string& model);
bool supportsCompletions(const std::string& model);
std::string getRecommendedApiForModel(const std::string& model);

} // namespace OpenAI 