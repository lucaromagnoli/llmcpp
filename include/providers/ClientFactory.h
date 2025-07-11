#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "core/LLMClient.h"

using json = nlohmann::json;

namespace llmcpp {

/**
 * Factory for creating LLM clients
 */
class ClientFactory {
   public:
    static std::unique_ptr<LLMClient> createClient(const std::string& provider, const json& config);
};

}  // namespace llmcpp
