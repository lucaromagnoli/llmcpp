#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/LLMClient.h"

using json = nlohmann::json;

namespace llmcpp {

/**
 * Manager for multiple LLM clients
 */
class ClientManager {
   public:
    ClientManager();

    void addClient(const std::string& name, const std::string& provider, const json& config);
    LLMClient* getClient(const std::string& name);
    std::vector<std::string> getClientNames() const;

   private:
    std::unordered_map<std::string, std::unique_ptr<LLMClient>> clients_;
};

}  // namespace llmcpp
