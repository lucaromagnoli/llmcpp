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
    if (!request.previousResponseId.empty()) {
        responsesReq.previousResponseID = request.previousResponseId;
    }

    // Handle tools
    if (request.config.tools.has_value()) {
        responsesReq.tools = request.config.tools.value();
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

// Implementation of ChatCompletionRequest::fromLLMRequest moved from header to avoid circular dependency
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

// Implementation of ChatCompletionRequest::toLLMRequest moved from header to avoid circular dependency
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

// Implementation of ChatCompletionResponse::toLLMResponse moved from header to avoid circular dependency
LLMResponse ChatCompletionResponse::toLLMResponse(bool expectStructuredOutput) const {
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

} // namespace OpenAI
