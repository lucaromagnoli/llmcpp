#include "openai/OpenAITypes.h"

#include "core/LLMTypes.h"  // Include for complete type definitions

namespace OpenAI {

// Implementation of ResponsesRequest::fromLLMRequest moved from header to avoid circular dependency
ResponsesRequest ResponsesRequest::fromLLMRequest(const LLMRequest& request) {
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

    // Handle tools from extensions (if present)
    if (hasTools(request.config)) {
        json toolsJson = getToolsJson(request.config);
        responsesReq.tools.clear();

        // Convert JSON tools array to ToolVariant vector
        for (const auto& toolJson : toolsJson) {
            if (toolJson.contains("type")) {
                std::string toolType = toolJson["type"].get<std::string>();
                if (toolType == "mcp") {
                    McpTool mcpTool = McpTool::fromJson(toolJson);
                    responsesReq.tools.push_back(mcpTool);
                }
            }
        }
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

// Implementation of ResponsesResponse::toLLMResponse moved from header to avoid circular dependency
LLMResponse ResponsesResponse::toLLMResponse(bool expectStructuredOutput) const {
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

// Implementation of ChatCompletionRequest::fromLLMRequest moved from header to avoid circular
// dependency
ChatCompletionRequest ChatCompletionRequest::fromLLMRequest(const LLMRequest& request) {
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

// Implementation of ChatCompletionRequest::toLLMRequest moved from header to avoid circular
// dependency
LLMRequest ChatCompletionRequest::toLLMRequest() const {
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

// Implementation of detectApiType moved from header to avoid circular dependency
ApiType detectApiType(const LLMRequest& request) {
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

bool ResponsesResponse::hasError() const {
    return status == ResponseStatus::Failed || status == ResponseStatus::Cancelled ||
           status == ResponseStatus::Incomplete;
}

std::string ResponsesResponse::getOutputText() const {
    std::string result;
    for (const auto& item : output) {
        // Check if this is a message with content
        if (item.contains("type") && item["type"] == "message" && item.contains("content")) {
            const auto& content = item["content"];
            if (content.is_array()) {
                for (const auto& contentItem : content) {
                    if (contentItem.contains("type") && contentItem["type"] == "output_text" &&
                        contentItem.contains("text")) {
                        if (!result.empty()) result += "\n";
                        result += contentItem["text"].get<std::string>();
                    }
                }
            }
        }
        // Also support simple text items (legacy format)
        else if (item.contains("type") && item["type"] == "text" && item.contains("text")) {
            if (!result.empty()) result += "\n";
            result += item["text"].get<std::string>();
        }
    }
    return result;
}

std::vector<FunctionCall> ResponsesResponse::getFunctionCalls() const {
    std::vector<FunctionCall> calls;
    for (const auto& item : output) {
        if (item.contains("type") && item["type"] == "function_call") {
            FunctionCall call;
            if (item.contains("name")) call.name = item["name"].get<std::string>();
            if (item.contains("arguments")) call.arguments = item["arguments"];
            calls.push_back(call);
        }
    }
    return calls;
}

std::vector<ImageGenerationCall> ResponsesResponse::getImageGenerations() const {
    std::vector<ImageGenerationCall> images;
    for (const auto& item : output) {
        if (item.contains("type") && item["type"] == "image") {
            ImageGenerationCall img;
            if (item.contains("url")) img.url = item["url"].get<std::string>();
            if (item.contains("prompt")) img.prompt = item["prompt"].get<std::string>();
            images.push_back(img);
        }
    }
    return images;
}

json ResponsesRequest::toJson() const {
    json j;
    j["model"] = model;

    if (input.has_value()) {
        j["input"] = input->toJson();
    }

    if (!include.empty()) {
        j["include"] = include;
    }

    if (!instructions.empty()) {
        j["instructions"] = instructions;
    }

    if (maxOutputTokens.has_value()) {
        j["max_output_tokens"] = maxOutputTokens.value();
    }

    if (text.has_value()) {
        j["text"] = text->toJson();
    }

    // Add tool choice
    if (toolChoice == ToolChoiceMode::None) {
        j["tool_choice"] = "none";
    } else if (toolChoice == ToolChoiceMode::Auto) {
        j["tool_choice"] = "auto";
    } else if (toolChoice == ToolChoiceMode::Required) {
        j["tool_choice"] = "required";
    }

    // Add tools
    if (!tools.empty()) {
        json toolsArray = json::array();
        for (const auto& tool : tools) {
            toolsArray.push_back(std::visit([](const auto& t) { return t.toJson(); }, tool));
        }
        j["tools"] = toolsArray;
    }

    if (topP.has_value()) {
        j["top_p"] = topP.value();
    }

    // Filter out temperature for reasoning models (o3, o3-mini)
    bool isReasoningModel = (model.find("o3") != std::string::npos);
    if (temperature.has_value() && !isReasoningModel) {
        j["temperature"] = temperature.value();
    }

    if (!user.empty()) {
        j["user"] = user;
    }

    j["store"] = store;

    if (!reasoningEffort.empty() && reasoningEffort != "medium") {
        j["reasoning_effort"] = reasoningEffort;
    }

    if (!metadata.empty()) {
        j["metadata"] = metadata;
    }

    if (reasoning.has_value()) {
        j["reasoning"] = reasoning.value();
    }

    return j;
}

ResponsesResponse ResponsesResponse::fromJson(const json& j) {
    ResponsesResponse resp;

    if (j.contains("id")) resp.id = j["id"].get<std::string>();
    if (j.contains("model")) resp.model = j["model"].get<std::string>();

    // Parse status
    if (j.contains("status")) {
        std::string statusStr = j["status"].get<std::string>();
        if (statusStr == "queued")
            resp.status = ResponseStatus::Queued;
        else if (statusStr == "in_progress")
            resp.status = ResponseStatus::InProgress;
        else if (statusStr == "completed")
            resp.status = ResponseStatus::Completed;
        else if (statusStr == "failed")
            resp.status = ResponseStatus::Failed;
        else if (statusStr == "cancelled")
            resp.status = ResponseStatus::Cancelled;
        else if (statusStr == "incomplete")
            resp.status = ResponseStatus::Incomplete;
    }

    if (j.contains("error")) resp.error = j["error"];
    if (j.contains("created_at")) resp.createdAt = j["created_at"].get<int>();
    // expiresAt field not in ResponsesResponse
    if (j.contains("metadata")) resp.metadata = j["metadata"];
    if (j.contains("reasoning_effort"))
        resp.reasoningEffort = j["reasoning_effort"].get<std::string>();

    // Parse usage - LLMUsage uses inputTokens/outputTokens
    if (j.contains("usage")) {
        const auto& usage = j["usage"];
        // Try both naming conventions
        if (usage.contains("input_tokens")) {
            resp.usage.inputTokens = usage["input_tokens"].get<int>();
        } else if (usage.contains("prompt_tokens")) {
            resp.usage.inputTokens = usage["prompt_tokens"].get<int>();
        }

        if (usage.contains("output_tokens")) {
            resp.usage.outputTokens = usage["output_tokens"].get<int>();
        } else if (usage.contains("completion_tokens")) {
            resp.usage.outputTokens = usage["completion_tokens"].get<int>();
        }
    }

    // Parse output
    if (j.contains("output") && j["output"].is_array()) {
        resp.output = j["output"];
    }

    return resp;
}

ChatCompletionChoice ChatCompletionChoice::fromJson(const json& j) {
    ChatCompletionChoice choice;

    if (j.contains("index")) choice.index = j["index"].get<int>();
    if (j.contains("finish_reason")) choice.finishReason = j["finish_reason"].get<std::string>();
    if (j.contains("logprobs")) choice.logprobs = j["logprobs"];

    // Parse message
    if (j.contains("message")) {
        const auto& msg = j["message"];
        if (msg.contains("role")) choice.message.role = msg["role"].get<std::string>();
        if (msg.contains("content")) choice.message.content = msg["content"].get<std::string>();
    }

    return choice;
}

// Helper functions for working with tools in LLMRequestConfig
void setTools(LLMRequestConfig& config, const std::vector<ToolVariant>& tools) {
    json toolsJson = json::array();
    for (const auto& tool : tools) {
        // Each tool type should have a toJson() method
        std::visit([&](const auto& t) { toolsJson.push_back(t.toJson()); }, tool);
    }
    config.extensions["tools"] = toolsJson;
}

bool hasTools(const LLMRequestConfig& config) {
    return config.extensions.contains("tools") && config.extensions["tools"].is_array() &&
           !config.extensions["tools"].empty();
}

json getToolsJson(const LLMRequestConfig& config) {
    if (config.extensions.contains("tools") && config.extensions["tools"].is_array()) {
        return config.extensions["tools"];
    }
    return json::array();
}

}  // namespace OpenAI
