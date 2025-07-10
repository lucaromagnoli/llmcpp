#include <doctest/doctest.h>
#include "core/LLMClient.h"

// Mock implementation for testing
class MockLLMClient : public LLMClient {
public:
    MockLLMClient() = default;
    
    LLMResponse sendRequest(const LLMRequest& request) override {
        LLMResponse response;
        response.success = true;
        response.result = nlohmann::json::object({{"mock", "response"}});
        response.responseId = "mock_id";
        return response;
    }
    
    std::future<LLMResponse> sendRequestAsync(const LLMRequest& request,
                                             LLMResponseCallback callback) override {
        return std::async(std::launch::async, [this, request, callback]() {
            auto response = sendRequest(request);
            if (callback) {
                callback(response);
            }
            return response;
        });
    }
    
    std::future<LLMResponse> sendStreamingRequest(const LLMRequest& request,
                                                 LLMStreamCallback streamCallback,
                                                 LLMResponseCallback finalCallback) override {
        return std::async(std::launch::async, [this, request, streamCallback, finalCallback]() {
            // Mock streaming
            if (streamCallback) {
                streamCallback("chunk1");
                streamCallback("chunk2");
                streamCallback("chunk3");
            }
            
            auto response = sendRequest(request);
            if (finalCallback) {
                finalCallback(response);
            }
            return response;
        });
    }
    
    void setApiKey(const std::string& apiKey) override {
        apiKey_ = apiKey;
    }
    
    std::string getApiKey() const override {
        return apiKey_.empty() ? "" : "***masked***";
    }
    
    bool isConfigured() const override {
        return !apiKey_.empty();
    }
    
    std::string getClientName() const override {
        return "mock";
    }
    
    std::vector<std::string> getAvailableModels() const override {
        return {"mock-model-1", "mock-model-2"};
    }
    
    bool isModelSupported(const std::string& modelName) const override {
        auto models = getAvailableModels();
        return std::find(models.begin(), models.end(), modelName) != models.end();
    }
    
    void setClientConfig(const json& config) override {
        clientConfig_ = config;
    }
    
    json getClientConfig() const override {
        return clientConfig_;
    }
    
private:
    std::string apiKey_;
    json clientConfig_;
};

TEST_CASE("LLMClient interface") {
    MockLLMClient client;
    
    SUBCASE("Initial state") {
        CHECK(client.getClientName() == "mock");
        CHECK_FALSE(client.isConfigured());
        CHECK(client.getApiKey().empty());
    }
    
    SUBCASE("API key management") {
        client.setApiKey("test_key");
        CHECK(client.isConfigured());
        CHECK(client.getApiKey() == "***masked***");
    }
    
    SUBCASE("Model support") {
        auto models = client.getAvailableModels();
        CHECK(models.size() == 2);
        CHECK(models[0] == "mock-model-1");
        CHECK(models[1] == "mock-model-2");
        
        CHECK(client.isModelSupported("mock-model-1"));
        CHECK(client.isModelSupported("mock-model-2"));
        CHECK_FALSE(client.isModelSupported("unsupported-model"));
    }
    
    SUBCASE("Client configuration") {
        json config = {{"key", "value"}, {"number", 42}};
        client.setClientConfig(config);
        
        auto retrievedConfig = client.getClientConfig();
        CHECK(retrievedConfig["key"] == "value");
        CHECK(retrievedConfig["number"] == 42);
    }
}

TEST_CASE("LLMClient requests") {
    MockLLMClient client;
    client.setApiKey("test_key");
    
    LLMRequestConfig config;
    config.client = "mock";
    config.model = "mock-model-1";
    
    LLMRequest request;
    request.config = config;
    request.prompt = "Test prompt";
    
    SUBCASE("Synchronous request") {
        auto response = client.sendRequest(request);
        
        CHECK(response.success);
        CHECK(response.result["mock"] == "response");
        CHECK(response.responseId == "mock_id");
    }
    
    SUBCASE("Asynchronous request") {
        bool callbackCalled = false;
        
        auto future = client.sendRequestAsync(request, [&callbackCalled](LLMResponse response) {
            callbackCalled = true;
            CHECK(response.success);
        });
        
        auto response = future.get();
        CHECK(response.success);
        CHECK(callbackCalled);
    }
    
    SUBCASE("Streaming request") {
        std::vector<std::string> chunks;
        bool finalCallbackCalled = false;
        
        auto future = client.sendStreamingRequest(request,
            [&chunks](const std::string& chunk) {
                chunks.push_back(chunk);
            },
            [&finalCallbackCalled](LLMResponse response) {
                finalCallbackCalled = true;
                CHECK(response.success);
            }
        );
        
        auto response = future.get();
        CHECK(response.success);
        CHECK(finalCallbackCalled);
        CHECK(chunks.size() == 3);
        CHECK(chunks[0] == "chunk1");
        CHECK(chunks[1] == "chunk2");
        CHECK(chunks[2] == "chunk3");
    }
} 