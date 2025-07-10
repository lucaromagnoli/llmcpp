#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "LLMClient.h"

/**
 * @brief Manager class for LLM clients
 *
 * This manager provides centralized client lifecycle management, connection pooling,
 * and thread-safe access to multiple LLM clients.
 */
class ClientManager {
   public:
    ClientManager() = default;
    ~ClientManager() = default;

    // Disable copy and move semantics (singleton-like behavior)
    ClientManager(const ClientManager&) = delete;
    ClientManager& operator=(const ClientManager&) = delete;
    ClientManager(ClientManager&&) = delete;
    ClientManager& operator=(ClientManager&&) = delete;

    /**
     * @brief Add a client to the manager
     *
     * @param name Unique name for the client
     * @param client The client to add
     * @return true if added successfully, false if name already exists
     */
    bool addClient(const std::string& name, std::unique_ptr<LLMClient> client);

    /**
     * @brief Get a client by name
     *
     * @param name The name of the client to retrieve
     * @return LLMClient* Pointer to the client, nullptr if not found
     */
    LLMClient* getClient(const std::string& name) const;

    /**
     * @brief Remove a client from the manager
     *
     * @param name The name of the client to remove
     * @return true if removed successfully, false if not found
     */
    bool removeClient(const std::string& name);

    /**
     * @brief Check if a client exists
     *
     * @param name The name of the client to check
     * @return true if the client exists, false otherwise
     */
    bool hasClient(const std::string& name) const;

    /**
     * @brief Get all client names
     *
     * @return std::vector<std::string> List of all client names
     */
    std::vector<std::string> getClientNames() const;

    /**
     * @brief Get the number of managed clients
     *
     * @return size_t Number of clients
     */
    size_t getClientCount() const;

    /**
     * @brief Clear all clients
     */
    void clear();

   private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::unique_ptr<LLMClient>> clients_;
};