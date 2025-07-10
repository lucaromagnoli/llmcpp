#pragma once
#include "core/LLMClient.h"
#include <memory>
#include <string>
#include <unordered_map>

/**
 * Manager for multiple LLM clients
 * TODO: Implement full client management
 */
class ClientManager {
public:
    ClientManager() = default;
    
    // Add a client
    void addClient(const std::string& name, std::unique_ptr<LLMClient> client);
    
    // Get a client by name
    LLMClient* getClient(const std::string& name);
    
    // Remove a client
    void removeClient(const std::string& name);
    
    // Get all client names
    std::vector<std::string> getClientNames() const;
    
    // Check if client exists
    bool hasClient(const std::string& name) const;
    
    // Clear all clients
    void clear();

private:
    std::unordered_map<std::string, std::unique_ptr<LLMClient>> clients_;
}; 