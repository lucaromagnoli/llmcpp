#include "providers/ClientManager.h"

void ClientManager::addClient(const std::string& name, std::unique_ptr<LLMClient> client) {
    clients_[name] = std::move(client);
}

LLMClient* ClientManager::getClient(const std::string& name) {
    auto it = clients_.find(name);
    return it != clients_.end() ? it->second.get() : nullptr;
}

void ClientManager::removeClient(const std::string& name) {
    clients_.erase(name);
}

std::vector<std::string> ClientManager::getClientNames() const {
    std::vector<std::string> names;
    names.reserve(clients_.size());
    for (const auto& pair : clients_) {
        names.push_back(pair.first);
    }
    return names;
}

bool ClientManager::hasClient(const std::string& name) const {
    return clients_.find(name) != clients_.end();
}

void ClientManager::clear() {
    clients_.clear();
} 