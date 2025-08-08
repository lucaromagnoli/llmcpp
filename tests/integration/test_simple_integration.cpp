#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "core/LLMTypes.h"
#include "openai/OpenAIClient.h"

// Simple integration tests for OpenAI API using .env files
// These tests are disabled by default to avoid accidental API usage and costs
// To enable: Set LLMCPP_RUN_INTEGRATION_TESTS=1 in .env file
// To set API key: Set OPENAI_API_KEY=your_key_here in .env file

// Simple .env file loader
void loadEnvFile(const std::string& filepath = ".env") {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return;  // File doesn't exist, that's okay
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        // Find the = separator
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove quotes if present
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                value = value.substr(1, value.size() - 2);
            }

            // Set environment variable (cross-platform)
#ifdef _WIN32
            _putenv_s(key.c_str(), value.c_str());
#else
            setenv(key.c_str(), value.c_str(), 1);
#endif
        }
    }
}

static bool shouldRunIntegrationTests() {
    const char* runTests = std::getenv("LLMCPP_RUN_INTEGRATION_TESTS");
    return runTests && std::string(runTests) == "1";
}

static std::string getApiKey() {
    const char* apiKey = std::getenv("OPENAI_API_KEY");
    return apiKey ? std::string(apiKey) : "";
}

TEST_CASE("Integration test info", "[integration][info]") {
    // Try to load .env file automatically
    loadEnvFile();

    std::cout << "\n🔧 To run OpenAI integration tests:\n" << std::endl;
    std::cout << "1. Create a .env file in the project root:" << std::endl;
    std::cout << "   echo 'OPENAI_API_KEY=sk-your-api-key-here' > .env" << std::endl;
    std::cout << "   echo 'LLMCPP_RUN_INTEGRATION_TESTS=1' >> .env" << std::endl;
    std::cout << "\n2. Run the tests:" << std::endl;
    std::cout << "   make test" << std::endl;
    std::cout << "\nAlternatively, use environment variables:" << std::endl;
    std::cout << "   export OPENAI_API_KEY='sk-your-api-key-here'" << std::endl;
    std::cout << "   export LLMCPP_RUN_INTEGRATION_TESTS=1" << std::endl;
    std::cout << "   make test" << std::endl;
    std::cout << "\n⚠️  Note: These tests make real API calls and will incur charges!" << std::endl;
    std::cout << "💡 Tip: Use gpt-4o-mini for cheaper testing" << std::endl;
    std::cout << "🔒 Security: Add .env to .gitignore!" << std::endl;

    REQUIRE(true);  // Always pass - this is just informational
}

TEST_CASE("OpenAI Integration - Simple request", "[openai][integration]") {
    // Load .env file automatically
    loadEnvFile();

    if (!shouldRunIntegrationTests()) {
        SKIP("Integration tests disabled. Set LLMCPP_RUN_INTEGRATION_TESTS=1 in .env file.");
        return;
    }

    std::string apiKey = getApiKey();
    if (apiKey.empty()) {
        SKIP("No API key found. Set OPENAI_API_KEY in .env file or environment.");
        return;
    }

    SECTION("Basic completion") {
        std::cout << "\n🚀 Testing real OpenAI API call..." << std::endl;

        OpenAIClient client(apiKey);

        LLMRequestConfig config;
        config.client = "openai";
        config.model = "gpt-4o-mini";  // Use cheaper model for testing
        // Do not set maxTokens explicitly; let server default decide
        config.temperature = 0.1f;     // Low temperature for consistency

        LLMRequest request(config, "Say 'Hello, World!' and nothing else.");

        std::cout << "📤 Request config: " << config.toString() << std::endl;
        std::cout << "📤 Request prompt: " << request.prompt << std::endl;

        auto response = client.sendRequest(request);

        std::cout << "Response received!" << std::endl;
        std::cout << "✅ Success: " << (response.success ? "true" : "false") << std::endl;
        std::cout << "📝 Response ID: " << response.responseId << std::endl;
        std::cout << "🔧 Usage: " << response.usage.toString() << std::endl;

        if (!response.success) {
            std::cout << "❌ Error: " << response.errorMessage << std::endl;
            std::cout << "📋 Full result: " << response.result.dump(2) << std::endl;
        } else {
            std::cout << "📋 Result: " << response.result.dump(2) << std::endl;
        }

        // Verify the response structure
        REQUIRE(response.success == true);
        REQUIRE(response.errorMessage.empty());
        REQUIRE(!response.responseId.empty());
        REQUIRE(response.usage.inputTokens > 0);
        REQUIRE(response.usage.outputTokens > 0);

        std::cout << "✅ All assertions passed!" << std::endl;
    }
}
