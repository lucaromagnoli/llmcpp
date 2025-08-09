#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "anthropic/AnthropicClient.h"
#include "anthropic/AnthropicTypes.h"

using namespace std::chrono;

TEST_CASE("Anthropic model benchmarks", "[anthropic][integration][benchmark]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    REQUIRE(apiKey != nullptr);

    Anthropic::AnthropicClient client(apiKey);

    // Test representative Claude models from different series
    std::vector<std::string> modelsToTest = {
        "claude-3-5-haiku-20241022",   // Fast and affordable
        "claude-3-5-sonnet-20241022",  // Balanced performance
        "claude-3-haiku-20240307"      // Legacy fast model
    };

    std::cout << "model,latency_ms,status,input_tokens,output_tokens,total_tokens" << std::endl;

    for (const auto& modelName : modelsToTest) {
        DYNAMIC_SECTION("Benchmark model: " << modelName) {
            LLMRequestConfig config;
            config.model = modelName;
            config.maxTokens = 50;      // Keep output consistent for benchmarking
            config.temperature = 0.1f;  // Low temperature for more deterministic results

            LLMRequest request(
                config, "Write a brief hello message in JSON format with a 'message' field.");

            const auto start = steady_clock::now();
            auto response = client.sendRequest(request);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();

            std::cout << modelName << "," << elapsedMs << "," << (response.success ? "ok" : "fail")
                      << "," << response.usage.inputTokens << "," << response.usage.outputTokens
                      << "," << response.usage.totalTokens() << std::endl;

            // Sanity check: we should get a response back
            REQUIRE(!response.result.empty());

            if (response.success) {
                REQUIRE(response.usage.inputTokens > 0);
                REQUIRE(response.usage.outputTokens > 0);
            }
        }
    }
}

TEST_CASE("Anthropic model comparison benchmarks",
          "[anthropic][integration][benchmark][comparison]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    REQUIRE(apiKey != nullptr);

    Anthropic::AnthropicClient client(apiKey);

    // Compare different model families on the same task
    struct ModelBenchmark {
        std::string model;
        std::string family;
        std::string expectedStrength;
    };

    std::vector<ModelBenchmark> benchmarks = {
        {"claude-3-5-haiku-20241022", "haiku", "speed"},
        {"claude-3-5-sonnet-20241022", "sonnet", "balanced"},
        {"claude-3-haiku-20240307", "legacy_haiku", "legacy_speed"}};

    std::cout << "\n=== Model Family Comparison ===" << std::endl;
    std::cout << "family,model,task,latency_ms,success,tokens_per_second" << std::endl;

    const std::string testPrompt =
        "Explain what artificial intelligence is in exactly 3 sentences.";

    for (const auto& benchmark : benchmarks) {
        DYNAMIC_SECTION("Compare " << benchmark.family << " - " << benchmark.model) {
            LLMRequestConfig config;
            config.model = benchmark.model;
            config.maxTokens = 100;
            config.temperature = 0.3f;

            LLMRequest request(config, testPrompt);

            const auto start = steady_clock::now();
            auto response = client.sendRequest(request);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();

            // Calculate tokens per second if successful
            double tokensPerSecond = 0.0;
            if (response.success && elapsedMs > 0) {
                tokensPerSecond = (response.usage.outputTokens * 1000.0) / elapsedMs;
            }

            std::cout << benchmark.family << "," << benchmark.model << ","
                      << "explanation," << elapsedMs << "," << (response.success ? "true" : "false")
                      << "," << tokensPerSecond << std::endl;

            // Verify the response makes sense
            if (response.success) {
                REQUIRE(response.result.contains("text"));
                auto text = response.result["text"].get<std::string>();
                REQUIRE(!text.empty());
                REQUIRE(response.usage.outputTokens > 10);  // Should be substantial response
            }
        }
    }
}

TEST_CASE("Anthropic context handling benchmarks", "[anthropic][integration][benchmark][context]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    REQUIRE(apiKey != nullptr);

    Anthropic::AnthropicClient client(apiKey);

    std::cout << "\n=== Context Handling Benchmark ===" << std::endl;
    std::cout << "context_size,model,latency_ms,input_tokens,output_tokens,success" << std::endl;

    // Test different context sizes
    std::vector<int> contextSizes = {0, 5, 10, 20};
    const std::string model = "claude-3-5-haiku-20241022";  // Use fast model for context tests

    for (int contextSize : contextSizes) {
        DYNAMIC_SECTION("Context size: " << contextSize) {
            LLMRequestConfig config;
            config.model = model;
            config.maxTokens = 50;

            // Build context
            LLMContext context;
            for (int i = 0; i < contextSize; ++i) {
                context.push_back({{"role", (i % 2 == 0) ? "user" : "assistant"},
                                   {"content", "Context message " + std::to_string(i + 1)}});
            }

            LLMRequest request(config, "Summarize our conversation.", context);

            const auto start = steady_clock::now();
            auto response = client.sendRequest(request);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();

            std::cout << contextSize << "," << model << "," << elapsedMs << ","
                      << response.usage.inputTokens << "," << response.usage.outputTokens << ","
                      << (response.success ? "true" : "false") << std::endl;

            // Verify context handling
            if (response.success) {
                REQUIRE(response.usage.inputTokens > contextSize * 2);  // Should scale with context
            }
        }
    }
}

TEST_CASE("Anthropic temperature benchmarks", "[anthropic][integration][benchmark][temperature]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* apiKey = std::getenv("ANTHROPIC_API_KEY");
    REQUIRE(apiKey != nullptr);

    Anthropic::AnthropicClient client(apiKey);

    std::cout << "\n=== Temperature Impact Benchmark ===" << std::endl;
    std::cout << "temperature,latency_ms,output_tokens,response_length" << std::endl;

    std::vector<float> temperatures = {0.1f, 0.5f, 0.9f};
    const std::string prompt = "Write a creative story opening in exactly 50 words.";

    for (float temp : temperatures) {
        DYNAMIC_SECTION("Temperature: " << temp) {
            // Use native Anthropic API for precise temperature control
            Anthropic::MessagesRequest request;
            request.model = "claude-3-5-haiku-20241022";
            request.maxTokens = 80;
            request.temperature = temp;

            Anthropic::Message userMsg;
            userMsg.role = Anthropic::MessageRole::USER;
            userMsg.content.push_back({.type = "text", .text = prompt});
            request.messages.push_back(userMsg);

            const auto start = steady_clock::now();
            auto response = client.sendMessagesRequest(request);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();

            // Get response length
            std::string fullText;
            for (const auto& content : response.content) {
                if (content.type == "text") {
                    fullText += content.text;
                }
            }

            std::cout << temp << "," << elapsedMs << "," << response.usage.outputTokens << ","
                      << fullText.length() << std::endl;

            // Verify response
            REQUIRE(!response.content.empty());
            REQUIRE(response.usage.outputTokens > 0);
        }
    }
}
