#include <llmcpp.h>

#include <cstdlib>
#include <iostream>

int main() {
    std::cout << "llmcpp Basic Usage Example" << std::endl;
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
        config.maxTokens = 100;
        config.temperature = 0.7f;

        // Create request
        LLMRequest request(config, "Hello! Can you tell me a fun fact about C++?");

        // Send request
        std::cout << "Sending request to OpenAI..." << std::endl;
        auto response = client.sendRequest(request);

        // Handle response
        if (response.success) {
            std::cout << "✅ Success!" << std::endl;
            std::cout << "Response: " << response.result["text"].get<std::string>() << std::endl;
            std::cout << "Usage: " << response.usage.toString() << std::endl;
        } else {
            std::cout << "❌ Error: " << response.errorMessage << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
