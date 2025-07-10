#pragma once
#include <string>
#include <vector>

/**
 * OpenAI model definitions and utilities
 * TODO: Add all OpenAI model definitions
 */
namespace OpenAIModels {
    // GPT-4 models
    extern const std::string GPT_4;
    extern const std::string GPT_4_TURBO;
    extern const std::string GPT_4O;
    extern const std::string GPT_4O_MINI;
    
    // GPT-3.5 models
    extern const std::string GPT_3_5_TURBO;
    
    // Get all available models
    std::vector<std::string> getAllModels();
    
    // Check if model supports function calling
    bool supportsTools(const std::string& model);
    
    // Check if model supports JSON mode
    bool supportsJsonMode(const std::string& model);
    
    // Get model context window size
    int getContextWindow(const std::string& model);
} 