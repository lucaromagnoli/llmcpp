#include "core/ClientManager.h"

#include <algorithm>
#include <mutex>

bool ClientManager::addClient(const std::string& name, std::unique_ptr<LLMClient> client) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (clients_.find(name) != clients_.end()) {
        return false;  // Client with this name already exists
    }

    clients_[name] = std::move(client);
    return true;
}

LLMClient* ClientManager::getClient(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = clients_.find(name);
    if (it != clients_.end()) {
        return it->second.get();
    }

    return nullptr;
}

bool ClientManager::removeClient(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = clients_.find(name);
    if (it != clients_.end()) {
        clients_.erase(it);
        return true;
    }

    return false;
}

bool ClientManager::hasClient(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.find(name) != clients_.end();
}

std::vector<std::string> ClientManager::getClientNames() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> names;
    names.reserve(clients_.size());

    std::transform(clients_.begin(), clients_.end(), std::back_inserter(names),
                   [](const auto& pair) { return pair.first; });

    return names;
}

size_t ClientManager::getClientCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return clients_.size();
}

void ClientManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    clients_.clear();
}