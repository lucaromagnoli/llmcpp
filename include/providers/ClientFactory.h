#pragma once
#include "core/LLMClient.h"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace llmcpp {

/**
 * Factory for creating LLM clients
 */
class ClientFactory {
public:
    static std::unique_ptr<LLMClient> createClient(const std::string& provider, const json& config);
};

} // namespace llmcpp 