#include <llmcpp.h>

#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <thread>

int main() {
    std::cout << "llmcpp Async Example" << std::endl;
    std::cout << "Library version: " << llmcpp::VERSION << std::endl;

    // Get API key from environment variable
    const char* apiKey = std::getenv("OPENAI_API_KEY");
    if (!apiKey) {
        std::cerr << "Error: OPENAI_API_KEY environment variable not set" << std::endl;
        std::cerr << "Please set your OpenAI API key: export OPENAI_API_KEY=your_key_here"
                  << std::endl;
        return 1;
    }

    try {
        // Create OpenAI client
        OpenAIClient client(apiKey);

        // Configure the request
        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";
        config.maxTokens = 50;
        config.randomness = 0.7f;

        // Create request
        LLMRequest request(config, "What's the weather like today?");

        // Send async request with callback
        std::cout << "Sending async request to OpenAI..." << std::endl;

        bool callbackCalled = false;
        auto future =
            client.sendRequestAsync(request, [&callbackCalled](const LLMResponse& response) {
                std::cout << "📞 Callback received!" << std::endl;
                if (response.success) {
                    std::cout << "✅ Async Success!" << std::endl;
                    std::cout << "Response: " << response.result["text"].get<std::string>()
                              << std::endl;
                } else {
                    std::cout << "❌ Async Error: " << response.errorMessage << std::endl;
                }
                callbackCalled = true;
            });

        // Do other work while waiting
        std::cout << "Doing other work while waiting for response..." << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "Working... " << (i + 1) << "/3" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // Get the final result
        auto response = future.get();
        std::cout << "🏁 Final result retrieved!" << std::endl;
        std::cout << "Usage: " << response.usage.toString() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}