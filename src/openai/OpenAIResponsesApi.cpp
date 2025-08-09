#include "openai/OpenAIResponsesApi.h"

#include <future>
#include <iostream>
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
        auto httpResponse = httpClient_->post(url, requestJson);

        if (!httpResponse.success) {
            std::cerr << "❌ HTTP request failed! Status: " << httpResponse.statusCode << std::endl;
            std::cerr << "❌ HTTP response body: " << httpResponse.body << std::endl;
            throw std::runtime_error("HTTP request failed: " + httpResponse.errorMessage);
        }

        // Parse the JSON response
        json responseJson = json::parse(httpResponse.body);

        // Extra debug for GPT-5 incomplete
        try {
            auto model = OpenAI::safeGetRequiredJson<std::string>(responseJson, "model");
            auto status = OpenAI::safeGetJson(responseJson, "status", std::string(""));
            if (model == "gpt-5" && status != "completed") {
                std::cerr << "⚠️ GPT-5 non-completed status: " << status << std::endl;
                if (responseJson.contains("incomplete_details")) {
                    std::cerr << "⚠️ Incomplete details: "
                              << responseJson["incomplete_details"].dump(2) << std::endl;
                }
            }
        } catch (...) {}

        // Check for API errors using safe JSON function
        auto error = OpenAI::safeGetOptionalJson<json>(responseJson, "error");
        if (error.has_value()) {
            handleApiError(responseJson);
        }

        // Process the successful response
        auto response = processResponse(responseJson);

        // Post-process the response
        postprocessResponse(response);

        return response;

    } catch (const json::exception& e) {
        std::cerr << "❌ JSON parsing error: " << e.what() << std::endl;
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
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
    const OpenAI::ResponsesRequest& request [[maybe_unused]],
    std::function<void(const std::string&)> streamCallback [[maybe_unused]],
    std::function<void(const OpenAI::ResponsesResponse&)> finalCallback [[maybe_unused]]) {
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

        // Check for API errors using safe JSON function
        auto error = OpenAI::safeGetOptionalJson<json>(responseJson, "error");
        if (error.has_value()) {
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

OpenAI::ResponsesResponse OpenAIResponsesApi::cancel(const std::string& responseId
                                                     [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::cancel not yet implemented");
}

bool OpenAIResponsesApi::deleteResponse(const std::string& responseId [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::deleteResponse not yet implemented");
}

json OpenAIResponsesApi::listInputItems(const std::string& responseId [[maybe_unused]],
                                        const std::string& after [[maybe_unused]],
                                        int limit [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::listInputItems not yet implemented");
}

bool OpenAIResponsesApi::isProcessing(const std::string& responseId [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::isProcessing not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::waitForCompletion(const std::string& responseId
                                                                [[maybe_unused]],
                                                                int timeoutSeconds [[maybe_unused]],
                                                                int pollIntervalSeconds
                                                                [[maybe_unused]]) {
    const int maxAttempts = std::max(1, timeoutSeconds / std::max(1, pollIntervalSeconds));
    return pollForCompletion(responseId, maxAttempts, pollIntervalSeconds);
}

std::future<OpenAI::ResponsesResponse> OpenAIResponsesApi::resumeStreaming(
    const std::string& responseId [[maybe_unused]], int startingAfter [[maybe_unused]],
    std::function<void(const std::string&)> streamCallback [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::resumeStreaming not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::continueConversation(
    const std::string& previousResponseId [[maybe_unused]],
    const OpenAI::ResponsesInput& newInput [[maybe_unused]],
    const std::optional<std::vector<OpenAI::ToolVariant>>& tools [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::continueConversation not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::forkConversation(
    const std::string& forkFromResponseId [[maybe_unused]],
    const OpenAI::ResponsesInput& newInput [[maybe_unused]],
    const std::optional<std::vector<OpenAI::ToolVariant>>& tools [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::forkConversation not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::approveMcpRequest(const std::string& responseId
                                                                [[maybe_unused]],
                                                                const std::string& approvalRequestId
                                                                [[maybe_unused]],
                                                                bool approve [[maybe_unused]]) {
    throw std::runtime_error("OpenAIResponsesApi::approveMcpRequest not yet implemented");
}

OpenAI::ResponsesResponse OpenAIResponsesApi::submitFunctionOutputs(
    const std::string& responseId [[maybe_unused]],
    const std::vector<OpenAI::FunctionCallOutput>& outputs [[maybe_unused]]) {
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
    if (request.input.has_value()) {
        try {
            auto inputJson = request.input->toJson();
            if (inputJson.is_null() ||
                (inputJson.is_string() && inputJson.get<std::string>().empty())) {
                errorMessage = "Input cannot be empty";
                return false;
            }
        } catch (const std::exception& e) {
            errorMessage = "Invalid input format: " + std::string(e.what());
            return false;
        }
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

bool OpenAIResponsesApi::supportsStreaming(const std::string& model [[maybe_unused]]) const {
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
std::string OpenAIResponsesApi::buildCreateUrl() const { return "/responses"; }

std::string OpenAIResponsesApi::buildRetrieveUrl(const std::string& responseId) const {
    return "/responses/" + responseId;
}

std::string OpenAIResponsesApi::buildCancelUrl(const std::string& responseId) const {
    return "/responses/" + responseId + "/cancel";
}

std::string OpenAIResponsesApi::buildDeleteUrl(const std::string& responseId) const {
    return "/responses/" + responseId;
}

std::string OpenAIResponsesApi::buildInputItemsUrl(const std::string& responseId) const {
    return "/responses/" + responseId + "/input_items";
}

void OpenAIResponsesApi::processStreamEvent(
    const std::string& event, std::function<void(const std::string&)> streamCallback) {
    if (streamCallback) {
        streamCallback(event);
    }
}

OpenAI::ResponsesResponse OpenAIResponsesApi::processResponse(const json& responseJson) {
    return OpenAI::ResponsesResponse::fromJson(responseJson);
}

void OpenAIResponsesApi::handleApiError(const json& errorResponse) const {
    auto errorObj = OpenAI::safeGetOptionalJson<json>(errorResponse, "error");
    if (errorObj.has_value()) {
        const auto& error = errorObj.value();
        std::string errorMsg = "OpenAI API Error";

        auto message = OpenAI::safeGetOptionalJson<std::string>(error, "message");
        if (message.has_value()) {
            errorMsg += ": " + message.value();
        }

        auto type = OpenAI::safeGetOptionalJson<std::string>(error, "type");
        if (type.has_value()) {
            errorMsg += " (Type: " + type.value() + ")";
        }

        auto code = OpenAI::safeGetOptionalJson<std::string>(error, "code");
        if (code.has_value()) {
            errorMsg += " (Code: " + code.value() + ")";
        }

        throw std::runtime_error(errorMsg);
    }

    throw std::runtime_error("Unknown API error: " + errorResponse.dump());
}

json OpenAIResponsesApi::preprocessRequest(const OpenAI::ResponsesRequest& request) const {
    auto requestJson = request.toJson();

    // Debug: Log the actual request being sent
    std::cout << "🔍 DEBUG: Actual JSON being sent to Responses API:" << std::endl;
    std::cout << requestJson.dump(2) << std::endl;

    return requestJson;
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

OpenAI::ResponsesResponse OpenAIResponsesApi::pollForCompletion(const std::string& responseId
                                                                [[maybe_unused]],
                                                                int maxAttempts [[maybe_unused]],
                                                                int intervalSeconds
                                                                [[maybe_unused]]) {
    for (int attempt = 0; attempt < maxAttempts; ++attempt) {
        auto resp = retrieve(responseId);
        if (resp.status == OpenAI::ResponseStatus::Completed ||
            resp.status == OpenAI::ResponseStatus::Failed ||
            resp.status == OpenAI::ResponseStatus::Cancelled) {
            return resp;
        }
        std::this_thread::sleep_for(std::chrono::seconds(std::max(1, intervalSeconds)));
    }
    // Final retrieve before giving up
    return retrieve(responseId);
}
