#include "openai/OpenAIResponsesApi.h"

#include <future>
#include <stdexcept>

#include "openai/OpenAIHttpClient.h"

OpenAIResponsesApi::OpenAIResponsesApi(std::shared_ptr<OpenAIHttpClient> httpClient)
    : httpClient_(std::move(httpClient)) {}

// Core Responses API methods
OpenAI::ResponsesResponse OpenAIResponsesApi::create(const OpenAI::ResponsesRequest& request) {
    try {
        // Preprocess the request
        json requestJson = preprocessRequest(request);

        // Add default parameters
        addDefaultParameters(requestJson);

        // Validate and process tools if present
        validateAndProcessTools(requestJson);

        // Make the HTTP request
        std::string url = buildCreateUrl();
        auto httpResponse = httpClient_->post(url, requestJson.dump());

        if (!httpResponse.success) {
            throw std::runtime_error("HTTP request failed: " + httpResponse.errorMessage);
        }

        // Parse the JSON response
        json responseJson = json::parse(httpResponse.body);

        // Check for API errors
        if (responseJson.contains("error")) {
            handleApiError(responseJson);
        }

        // Process the successful response
        auto response = processResponse(responseJson);

        // Post-process the response
        postprocessResponse(response);

        return response;

    } catch (const json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("API call failed: " + std::string(e.what()));
    }
}

std::future<OpenAI::ResponsesResponse> OpenAIResponsesApi::createAsync(
    const OpenAI::ResponsesRequest& request,
    std::function<void(const OpenAI::ResponsesResponse&)> callback) {
    return std::async(std::launch::async, [this, request, callback]() {
        try {
            auto response = create(request);
            if (callback) {
                callback(response);
            }
            return response;
        } catch (const std::exception& e) {
            // Create error response
            OpenAI::ResponsesResponse errorResponse;
            errorResponse.status = OpenAI::ResponseStatus::Failed;
            errorResponse.error = json{{"message", e.what()}};

            if (callback) {
                callback(errorResponse);
            }
            return errorResponse;
        }
    });
}

std::future<OpenAI::ResponsesResponse> OpenAIResponsesApi::createStreaming(
    const OpenAI::ResponsesRequest& request, std::function<void(const std::string&)> streamCallback,
    std::function<void(const OpenAI::ResponsesResponse&)> finalCallback) {
    // TODO: Implement streaming API call
    throw std::runtime_error("OpenAIResponsesApi::createStreaming not yet implemented");
}

// Response management methods
OpenAI::ResponsesResponse OpenAIResponsesApi::retrieve(const std::string& responseId) {
    try {
        std::string url = buildRetrieveUrl(responseId);
        auto httpResponse = httpClient_->get(url);

        if (!httpResponse.success) {
            throw std::runtime_error("HTTP request failed: " + httpResponse.errorMessage);
        }

        json responseJson = json::parse(httpResponse.body);

        if (responseJson.contains("error")) {
            handleApiError(responseJson);
        }

        auto response = processResponse(responseJson);
        postprocessResponse(response);

        return response;

    } catch (const json::exception& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Retrieve failed: " + std::string(e.what()));
    }
}

OpenAI::ResponsesResponse OpenAIResponsesApi::cancel(const std::string& responseId) {
    // TODO: Implement cancel
    throw std::runtime_error("OpenAIResponsesApi::cancel not yet implemented");
}

bool OpenAIResponsesApi::deleteResponse(const std::string& responseId) {
    // TODO: Implement delete
    throw std::runtime_error("OpenAIResponsesApi::deleteResponse not yet implemented");
}

json OpenAIResponsesApi::listInputItems(const std::string& responseId, const std::string& after,
                                        int limit) {
    // TODO: Implement list input items
    throw std::runtime_error("OpenAIResponsesApi::listInputItems not yet implemented");
}

// Background task support
bool OpenAIResponsesApi::isProcessing(const std::string& responseId) {
    // TODO: Implement processing check
    throw std::runtime_error("OpenAIResponsesApi::isProcessing not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::waitForCompletion(const std::string& responseId,
                                                                int timeoutSeconds,
                                                                int pollIntervalSeconds) {
    // TODO: Implement wait for completion
    throw std::runtime_error("OpenAIResponsesApi::waitForCompletion not yet implemented");
}

// Streaming helpers
std::future<OpenAI::ResponsesResponse> OpenAIResponsesApi::resumeStreaming(
    const std::string& responseId, int startingAfter,
    std::function<void(const std::string&)> streamCallback) {
    // TODO: Implement resume streaming
    throw std::runtime_error("OpenAIResponsesApi::resumeStreaming not yet implemented");
}

// Conversation management
OpenAI::ResponsesResponse OpenAIResponsesApi::continueConversation(
    const std::string& previousResponseId, const OpenAI::ResponsesInput& newInput,
    const std::optional<std::vector<OpenAI::ToolVariant>>& tools) {
    // TODO: Implement continue conversation
    throw std::runtime_error("OpenAIResponsesApi::continueConversation not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::forkConversation(
    const std::string& forkFromResponseId, const OpenAI::ResponsesInput& newInput,
    const std::optional<std::vector<OpenAI::ToolVariant>>& tools) {
    // TODO: Implement fork conversation
    throw std::runtime_error("OpenAIResponsesApi::forkConversation not yet implemented");
}

// Tool and approval management
OpenAI::ResponsesResponse OpenAIResponsesApi::approveMcpRequest(
    const std::string& responseId, const std::string& approvalRequestId, bool approve) {
    // TODO: Implement MCP approval
    throw std::runtime_error("OpenAIResponsesApi::approveMcpRequest not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::submitFunctionOutputs(
    const std::string& responseId, const std::vector<OpenAI::FunctionCallOutput>& outputs) {
    // TODO: Implement function output submission
    throw std::runtime_error("OpenAIResponsesApi::submitFunctionOutputs not yet implemented");
}

// Configuration and validation
bool OpenAIResponsesApi::validateRequest(const OpenAI::ResponsesRequest& request,
                                         std::string& errorMessage) const {
    // Check required fields
    if (request.model.empty()) {
        errorMessage = "Model is required";
        return false;
    }

    // Validate model is supported
    auto supportedModels = getSupportedModels();
    if (std::find(supportedModels.begin(), supportedModels.end(), request.model) ==
        supportedModels.end()) {
        errorMessage = "Model '" + request.model + "' is not supported for Responses API";
        return false;
    }

    // Validate input
    try {
        auto inputJson = request.input.toJson();
        if (inputJson.is_null() ||
            (inputJson.is_string() && inputJson.get<std::string>().empty())) {
            errorMessage = "Input cannot be empty";
            return false;
        }
    } catch (const std::exception& e) {
        errorMessage = "Invalid input format: " + std::string(e.what());
        return false;
    }

    // Validate optional parameters
    if (request.maxOutputTokens && *request.maxOutputTokens <= 0) {
        errorMessage = "max_output_tokens must be positive";
        return false;
    }

    if (request.temperature && (*request.temperature < 0.0 || *request.temperature > 2.0)) {
        errorMessage = "temperature must be between 0.0 and 2.0";
        return false;
    }

    if (request.topP && (*request.topP <= 0.0 || *request.topP > 1.0)) {
        errorMessage = "top_p must be between 0.0 and 1.0";
        return false;
    }

    // Validate tools if present
    if (request.tools) {
        for (const auto& tool : *request.tools) {
            try {
                std::visit([](const auto& t) { t.toJson(); }, tool);
            } catch (const std::exception& e) {
                errorMessage = "Invalid tool configuration: " + std::string(e.what());
                return false;
            }
        }
    }

    return true;
}

std::vector<std::string> OpenAIResponsesApi::getSupportedModels() const {
    return OpenAI::RESPONSES_MODELS;
}

bool OpenAIResponsesApi::supportsBackgroundProcessing(const std::string& model) const {
    // Most reasoning models support background processing
    return model.find("o1") != std::string::npos || model.find("o3") != std::string::npos ||
           model.find("o4") != std::string::npos;
}

bool OpenAIResponsesApi::supportsStreaming(const std::string& model) const {
    // Most models support streaming
    return true;
}

bool OpenAIResponsesApi::supportsTools(const std::string& model) const {
    // Most modern models support tools
    return model.find("gpt-4") != std::string::npos || model.find("o1") != std::string::npos ||
           model.find("o3") != std::string::npos || model.find("o4") != std::string::npos;
}

bool OpenAIResponsesApi::supportsImageGeneration(const std::string& model) const {
    return model == "gpt-image-1";
}

bool OpenAIResponsesApi::supportsCodeInterpreter(const std::string& model) const {
    // Most reasoning and advanced models support code interpreter
    return supportsTools(model);
}

bool OpenAIResponsesApi::supportsWebSearch(const std::string& model) const {
    // Most models support web search through the Responses API
    return supportsTools(model);
}

bool OpenAIResponsesApi::supportsMcp(const std::string& model) const {
    // MCP is supported by newer models
    return model.find("gpt-4.1") != std::string::npos || model.find("o3") != std::string::npos ||
           model.find("o4") != std::string::npos;
}

// Private helper methods implementation
std::string OpenAIResponsesApi::buildCreateUrl() const { return "/v1/responses"; }

std::string OpenAIResponsesApi::buildRetrieveUrl(const std::string& responseId) const {
    return "/v1/responses/" + responseId;
}

std::string OpenAIResponsesApi::buildCancelUrl(const std::string& responseId) const {
    return "/v1/responses/" + responseId + "/cancel";
}

std::string OpenAIResponsesApi::buildDeleteUrl(const std::string& responseId) const {
    return "/v1/responses/" + responseId;
}

std::string OpenAIResponsesApi::buildInputItemsUrl(const std::string& responseId) const {
    return "/v1/responses/" + responseId + "/input_items";
}

void OpenAIResponsesApi::processStreamEvent(
    const std::string& event, std::function<void(const std::string&)> streamCallback) {
    // TODO: Implement stream event processing for streaming responses
    if (streamCallback) {
        streamCallback(event);
    }
}

OpenAI::ResponsesResponse OpenAIResponsesApi::processResponse(const json& responseJson) {
    return OpenAI::ResponsesResponse::fromJson(responseJson);
}

void OpenAIResponsesApi::handleApiError(const json& errorResponse) const {
    if (errorResponse.contains("error")) {
        const auto& error = errorResponse["error"];
        std::string errorMsg = "OpenAI API Error";

        if (error.contains("message")) {
            errorMsg += ": " + error["message"].get<std::string>();
        }

        if (error.contains("type")) {
            errorMsg += " (Type: " + error["type"].get<std::string>() + ")";
        }

        if (error.contains("code")) {
            errorMsg += " (Code: " + error["code"].get<std::string>() + ")";
        }

        throw std::runtime_error(errorMsg);
    }

    throw std::runtime_error("Unknown API error: " + errorResponse.dump());
}

json OpenAIResponsesApi::preprocessRequest(const OpenAI::ResponsesRequest& request) const {
    return request.toJson();
}

void OpenAIResponsesApi::addDefaultParameters(json& requestJson) const {
    // Set default values if not already specified
    if (!requestJson.contains("stream")) {
        requestJson["stream"] = false;  // Default to non-streaming
    }

    if (!requestJson.contains("store")) {
        requestJson["store"] = true;  // Default to storing responses
    }

    // Ensure model is set
    if (!requestJson.contains("model") || requestJson["model"].empty()) {
        requestJson["model"] = "gpt-4o";  // Default model
    }
}

void OpenAIResponsesApi::validateAndProcessTools(json& requestJson) const {
    if (requestJson.contains("tools") && requestJson["tools"].is_array()) {
        // Basic validation that tools array is properly formatted
        for (const auto& tool : requestJson["tools"]) {
            if (!tool.contains("type")) {
                throw std::invalid_argument("Tool missing required 'type' field");
            }

            std::string toolType = tool["type"].get<std::string>();
            if (toolType == "function" && !tool.contains("name")) {
                throw std::invalid_argument("Function tool missing required 'name' field");
            }
        }
    }
}

void OpenAIResponsesApi::postprocessResponse(OpenAI::ResponsesResponse& response) const {
    // Extract convenience fields like outputText if not already set
    extractConvenienceFields(response);
}

void OpenAIResponsesApi::extractConvenienceFields(OpenAI::ResponsesResponse& response) const {
    // Extract outputText convenience field if not already set
    if (!response.outputText.has_value() || response.outputText->empty()) {
        response.outputText = response.getOutputText();
    }
}

OpenAI::ResponsesResponse OpenAIResponsesApi::pollForCompletion(const std::string& responseId,
                                                                int maxAttempts,
                                                                int intervalSeconds) {
    // TODO: Implement polling
    throw std::runtime_error("OpenAIResponsesApi::pollForCompletion not yet implemented");
}