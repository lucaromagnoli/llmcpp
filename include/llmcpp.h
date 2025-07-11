#pragma once

/**
 * @file llmcpp.h
 * @brief Main header file for the llmcpp library
 *
 * This header provides a unified interface for Large Language Model APIs
 * with support for multiple providers, async operations, and modern C++20 features.
 */

// Core functionality
#include "core/ClientManager.h"
#include "core/JsonSchemaBuilder.h"
#include "core/LLMClient.h"
#include "core/LLMTypes.h"

// OpenAI provider
#include "openai/OpenAIClient.h"
#include "openai/OpenAISchemaBuilder.h"
#include "openai/OpenAITypes.h"

// Version information
#include "llmcpp_version.h"

// Provider factory
#include "providers/ClientFactory.h"

/**
 * @namespace llmcpp
 * @brief Main namespace for the llmcpp library
 *
 * Contains all public API classes and functions for interacting with
 * Large Language Model providers.
 */
namespace llmcpp {

// Export main types for convenience
using LLMClient = ::LLMClient;
using LLMRequest = ::LLMRequest;
using LLMResponse = ::LLMResponse;
using LLMRequestConfig = ::LLMRequestConfig;
using LLMUsage = ::LLMUsage;
using ClientManager = ::ClientManager;

// OpenAI types
using OpenAIClient = ::OpenAIClient;

}  // namespace llmcpp
