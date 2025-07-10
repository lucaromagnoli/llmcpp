#include "providers/ClientManager.h"
#include "providers/ClientFactory.h"
#include <stdexcept>

namespace llmcpp {

ClientManager::ClientManager() = default;

void ClientManager::addClient(const std::string& name, const std::string& provider, const json& config) {
    auto client = ClientFactory::createClient(provider, config);
    clients_[name] = std::move(client);
}

LLMClient* ClientManager::getClient(const std::string& name) {
    auto it = clients_.find(name);
    if (it != clients_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<std::string> ClientManager::getClientNames() const {
    std::vector<std::string> names;
    for (const auto& [name, client] : clients_) {
        names.push_back(name);
    }
    return names;
}

} // namespace llmcpp 