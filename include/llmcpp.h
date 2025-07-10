#pragma once

/**
 * llmcpp - Modern C++20 Library for Large Language Model APIs
 * 
 * A unified interface for interacting with Large Language Model APIs
 * with support for async operations, streaming, and multiple providers.
 * 
 * Copyright (c) 2025 Nomad Monad
 * Licensed under the MIT License
 */

// Core types and interfaces
#include "core/LLMTypes.h"
#include "core/LLMClient.h"

// Provider implementations
#include "openai/OpenAIClient.h"
#include "openai/OpenAIModels.h"
#include "openai/OpenAIUtils.h"

// Provider management
#include "providers/ClientFactory.h"
#include "providers/ClientManager.h"

namespace llmcpp {
    // Library version
    constexpr const char* VERSION = "1.0.0";
    
    // Convenience aliases
    using Request = LLMRequest;
    using Response = LLMResponse;
    using Config = LLMRequestConfig;
    using Client = LLMClient;
    using ErrorCode = LLMErrorCode;
    using Usage = LLMUsage;
    
    // Callback aliases
    using ResponseCallback = LLMResponseCallback;
    using StreamCallback = LLMStreamCallback;
}

/**
 * Quick start example:
 * 
 * #include <llmcpp.h>
 * 
 * auto client = llmcpp::ClientFactory::createOpenAIClient("your-api-key");
 * 
 * llmcpp::Config config;
 * config.client = "openai";
 * config.model = "gpt-4";
 * config.maxTokens = 100;
 * 
 * llmcpp::Request request;
 * request.config = config;
 * request.prompt = "Hello, how are you?";
 * 
 * auto response = client->sendRequest(request);
 * if (response.success) {
 *     std::cout << response.result.dump() << std::endl;
 * }
 */ 