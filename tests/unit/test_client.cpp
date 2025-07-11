#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <future>

#include "core/LLMClient.h"

// Mock implementation for testing
class MockLLMClient : public LLMClient {
   public:
    MockLLMClient() = default;

    // Implementation of the pure virtual method from LLMClient
    void sendRequest(const LLMRequest& request, LLMResponseCallback callback) override {
        LLMResponse response;
        response.success = true;
        response.result = json::object({{"mock", "response"}});
        response.responseId = "mock_id";

        if (callback) {
            callback(response);
        }
    }

    // Additional synchronous method for testing convenience
    LLMResponse sendRequestSync(const LLMRequest& request) {
        LLMResponse response;
        response.success = true;
        response.result = json::object({{"mock", "response"}});
        response.responseId = "mock_id";
        return response;
    }

    std::future<LLMResponse> sendRequestAsync(const LLMRequest& request,
                                              LLMResponseCallback callback) {
        return std::async(std::launch::async, [this, request, callback]() {
            auto response = sendRequestSync(request);
            if (callback) {
                callback(response);
            }
            return response;
        });
    }

    std::future<LLMResponse> sendStreamingRequest(const LLMRequest& request,
                                                  LLMStreamCallback streamCallback,
                                                  LLMResponseCallback finalCallback) {
        return std::async(std::launch::async, [this, request, streamCallback, finalCallback]() {
            // Mock streaming
            if (streamCallback) {
                streamCallback("chunk1");
                streamCallback("chunk2");
                streamCallback("chunk3");
            }

            auto response = sendRequestSync(request);
            if (finalCallback) {
                finalCallback(response);
            }
            return response;
        });
    }

    void setApiKey(const std::string& apiKey) { apiKey_ = apiKey; }

    std::string getApiKey() const { return apiKey_.empty() ? "" : "***masked***"; }

    bool isConfigured() const { return !apiKey_.empty(); }

    std::string getClientName() const override { return "mock"; }

    std::vector<std::string> getAvailableModels() const override {
        return {"mock-model-1", "mock-model-2"};
    }

    bool isModelSupported(const std::string& modelName) const {
        auto models = getAvailableModels();
        return std::find(models.begin(), models.end(), modelName) != models.end();
    }

    void setClientConfig(const json& config) { clientConfig_ = config; }

    json getClientConfig() const { return clientConfig_; }

   private:
    std::string apiKey_;
    json clientConfig_;
};

TEST_CASE("LLMClient interface") {
    MockLLMClient client;

    SECTION("Initial state") {
        REQUIRE(client.getClientName() == "mock");
        REQUIRE_FALSE(client.isConfigured());
        REQUIRE(client.getApiKey().empty());
    }

    SECTION("API key management") {
        client.setApiKey("test_key");
        REQUIRE(client.isConfigured());
        REQUIRE(client.getApiKey() == "***masked***");
    }

    SECTION("Model support") {
        auto models = client.getAvailableModels();
        REQUIRE(models.size() == 2);
        REQUIRE(models[0] == "mock-model-1");
        REQUIRE(models[1] == "mock-model-2");

        REQUIRE(client.isModelSupported("mock-model-1"));
        REQUIRE(client.isModelSupported("mock-model-2"));
        REQUIRE_FALSE(client.isModelSupported("unsupported-model"));
    }

    SECTION("Client configuration") {
        json config = {{"key", "value"}, {"number", 42}};
        client.setClientConfig(config);

        auto retrievedConfig = client.getClientConfig();
        REQUIRE(retrievedConfig["key"] == "value");
        REQUIRE(retrievedConfig["number"] == 42);
    }
}

TEST_CASE("LLMClient requests") {
    MockLLMClient client;
    client.setApiKey("test_key");

    LLMRequestConfig config;
    config.client = "mock";
    config.model = "mock-model-1";

    LLMRequest request(config, "Test prompt");

    SECTION("Synchronous request") {
        auto response = client.sendRequestSync(request);

        REQUIRE(response.success);
        REQUIRE(response.result["mock"] == "response");
        REQUIRE(response.responseId == "mock_id");
    }

    SECTION("Asynchronous request") {
        bool callbackCalled = false;

        auto future = client.sendRequestAsync(request, [&callbackCalled](LLMResponse response) {
            callbackCalled = true;
            REQUIRE(response.success);
        });

        auto response = future.get();
        REQUIRE(response.success);
        REQUIRE(callbackCalled);
    }

    SECTION("Streaming request") {
        std::vector<std::string> chunks;
        bool finalCallbackCalled = false;

        auto future = client.sendStreamingRequest(
            request, [&chunks](const std::string& chunk) { chunks.push_back(chunk); },
            [&finalCallbackCalled](LLMResponse response) {
                finalCallbackCalled = true;
                REQUIRE(response.success);
            });

        auto response = future.get();
        REQUIRE(response.success);
        REQUIRE(finalCallbackCalled);
        REQUIRE(chunks.size() == 3);
        REQUIRE(chunks[0] == "chunk1");
        REQUIRE(chunks[1] == "chunk2");
        REQUIRE(chunks[2] == "chunk3");
    }
}