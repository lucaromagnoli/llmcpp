#pragma once
#include <string>
#include <vector>

#include "openai/OpenAITypes.h"

namespace OpenAI {

/**
 * Utility functions for OpenAI models
 */
std::vector<std::string> getAllModels();

}  // namespace OpenAI