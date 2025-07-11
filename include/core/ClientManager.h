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
 * Uses shared_ptr for safe memory management and shared ownership.
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
     * @param client The client to add (shared_ptr for shared ownership)
     * @return true if added successfully, false if name already exists
     */
    bool addClient(const std::string& name, std::shared_ptr<LLMClient> client);

    /**
     * @brief Get a client by name
     *
     * @param name The name of the client to retrieve
     * @return std::shared_ptr<LLMClient> Shared pointer to the client, nullptr if not found
     */
    std::shared_ptr<LLMClient> getClient(const std::string& name) const;

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

    /**
     * @brief Helper to create and add a client of specific type
     *
     * @tparam ClientType The type of client to create (must inherit from LLMClient)
     * @tparam Args Constructor argument types
     * @param name Unique name for the client
     * @param args Arguments to forward to the client constructor
     * @return std::shared_ptr<ClientType> Pointer to the created client, nullptr if name exists
     */
    template <typename ClientType, typename... Args>
    std::shared_ptr<ClientType> createClient(const std::string& name, Args&&... args) {
        static_assert(std::is_base_of_v<LLMClient, ClientType>,
                      "ClientType must inherit from LLMClient");

        auto client = std::make_shared<ClientType>(std::forward<Args>(args)...);
        if (addClient(name, client)) {
            return client;
        }
        return nullptr;
    }

   private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<LLMClient>> clients_;
};