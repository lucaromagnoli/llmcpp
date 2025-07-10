#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace OpenAI {

/**
 * Utility functions for OpenAI API
 */
std::string buildUrl(const std::string& endpoint);
json createHeaders(const std::string& apiKey);

} // namespace OpenAI 