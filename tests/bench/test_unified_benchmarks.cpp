#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "anthropic/AnthropicClient.h"
#include "anthropic/AnthropicTypes.h"
#include "openai/OpenAIClient.h"
#include "openai/OpenAITypes.h"

using namespace std::chrono;

// Helper functions for OpenAI models
static bool isReasoningModel(OpenAI::Model model) {
    return model == OpenAI::Model::GPT_5 || model == OpenAI::Model::GPT_5_Mini ||
           model == OpenAI::Model::GPT_5_Nano || model == OpenAI::Model::O3 ||
           model == OpenAI::Model::O3_Mini || model == OpenAI::Model::O1 ||
           model == OpenAI::Model::O1_Mini || model == OpenAI::Model::O1_Preview ||
           model == OpenAI::Model::O1_Pro || model == OpenAI::Model::O4_Mini;
}

static bool isExcludedModel(const std::string& modelName) {
    return modelName == "gpt-image-1" || modelName == "computer-use-preview";
}

// Benchmark result structure
struct BenchmarkResult {
    std::string provider;
    std::string model;
    std::string task;
    int64_t latencyMs;
    bool success;
    int inputTokens;
    int outputTokens;
    int totalTokens;
    double tokensPerSecond;

    std::string formatCSV() const {
        std::ostringstream ss;
        ss << provider << "," << model << "," << task << "," << latencyMs << ","
           << (success ? "ok" : "fail") << "," << inputTokens << "," << outputTokens << ","
           << totalTokens << "," << std::fixed << std::setprecision(2) << tokensPerSecond;
        return ss.str();
    }

    std::string formatTable() const {
        std::ostringstream ss;
        ss << std::left << std::setw(12) << provider << std::setw(25) << model << std::setw(12)
           << task << std::right << std::setw(8) << latencyMs << "ms" << std::setw(8)
           << (success ? "✅" : "❌") << std::setw(6) << inputTokens << std::setw(6) << outputTokens
           << std::setw(8) << std::fixed << std::setprecision(1) << tokensPerSecond;
        return ss.str();
    }
};

TEST_CASE("Unified OpenAI vs Anthropic Benchmarks", "[unified][benchmark][openai][anthropic]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* openaiKey = std::getenv("OPENAI_API_KEY");
    const char* anthropicKey = std::getenv("ANTHROPIC_API_KEY");

    std::vector<BenchmarkResult> results;

    std::cout << "\n=== UNIFIED MODEL BENCHMARKS ===" << std::endl;
    std::cout << "provider,model,task,latency_ms,status,input_tokens,output_tokens,total_tokens,"
                 "tokens_per_second"
              << std::endl;

    // Test prompt for simple text generation
    const std::string simplePrompt = "Write a brief 'Hello World' message.";

    // Test prompt for structured output
    const std::string structuredPrompt =
        "Respond with JSON: {\"message\": \"Hello\", \"status\": \"success\"}";

    SECTION("Simple Text Generation Benchmark") {
        // Anthropic Models
        if (anthropicKey) {
            std::vector<std::pair<std::string, Anthropic::Model>> anthropicModels = {
                {"claude-3-5-haiku", Anthropic::Model::CLAUDE_HAIKU_3_5},
                {"claude-3-5-sonnet-v2", Anthropic::Model::CLAUDE_SONNET_3_5_V2},
                {"claude-opus-4-1", Anthropic::Model::CLAUDE_OPUS_4_1},
                {"claude-sonnet-4", Anthropic::Model::CLAUDE_SONNET_4}};

            for (const auto& [shortName, modelEnum] : anthropicModels) {
                try {
                    Anthropic::AnthropicClient client(anthropicKey, modelEnum);

                    LLMRequestConfig config;
                    config.model = Anthropic::toString(modelEnum);
                    config.maxTokens = 50;
                    config.temperature = 0.1f;

                    LLMRequest request(config, simplePrompt);

                    const auto start = steady_clock::now();
                    auto response = client.sendRequest(request);
                    const auto end = steady_clock::now();

                    const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
                    const double tokensPerSec =
                        elapsedMs > 0 ? (response.usage.totalTokens() * 1000.0) / elapsedMs : 0.0;

                    BenchmarkResult result = {"Anthropic",
                                              shortName,
                                              "simple",
                                              elapsedMs,
                                              response.success,
                                              response.usage.inputTokens,
                                              response.usage.outputTokens,
                                              response.usage.totalTokens(),
                                              tokensPerSec};

                    results.push_back(result);
                    std::cout << result.formatCSV() << std::endl;

                } catch (const std::exception& e) {
                    BenchmarkResult result = {"Anthropic", shortName, "simple", 0,  false,
                                              0,           0,         0,        0.0};
                    results.push_back(result);
                    std::cout << result.formatCSV() << " # Error: " << e.what() << std::endl;
                }
            }
        }

        // OpenAI Models (only Responses API compatible models)
        if (openaiKey) {
            std::vector<std::pair<std::string, std::string>> openaiModels = {
                {"gpt-4o", "gpt-4o"},
                {"gpt-4o-mini", "gpt-4o-mini"},
                {"gpt-5", "gpt-5"},
                {"gpt-5-mini", "gpt-5-mini"},
                {"o3", "o3"},
                {"o3-mini", "o3-mini"},
                {"o1", "o1"},
                {"o1-mini", "o1-mini"},
                {"o1-preview", "o1-preview"}};

            OpenAIClient client(openaiKey);

            for (const auto& [shortName, modelName] : openaiModels) {
                if (isExcludedModel(modelName)) continue;

                try {
                    // Use Responses API for ALL models
                    auto input = OpenAI::ResponsesInput::fromText(simplePrompt);

                    OpenAI::ResponsesRequest req;
                    req.model = modelName;
                    req.input = input;

                    auto modelEnum = OpenAI::modelFromString(modelName);
                    if (isReasoningModel(modelEnum)) {
                        req.reasoning = json{{"effort", "low"}};
                    }

                    const auto start = steady_clock::now();
                    auto response = client.sendResponsesRequest(req);
                    const auto end = steady_clock::now();

                    const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
                    const int totalTokens =
                        response.usage.inputTokens + response.usage.outputTokens;
                    const double tokensPerSec =
                        elapsedMs > 0 ? (totalTokens * 1000.0) / elapsedMs : 0.0;

                    BenchmarkResult result = {"OpenAI",
                                              shortName,
                                              "simple",
                                              elapsedMs,
                                              response.isCompleted() && !response.hasError(),
                                              response.usage.inputTokens,
                                              response.usage.outputTokens,
                                              totalTokens,
                                              tokensPerSec};

                    results.push_back(result);
                    std::cout << result.formatCSV() << std::endl;

                } catch (const std::exception& e) {
                    BenchmarkResult result = {"OpenAI", shortName, "simple", 0,  false,
                                              0,        0,         0,        0.0};
                    results.push_back(result);
                    std::cout << result.formatCSV() << " # Error: " << e.what() << std::endl;
                }
            }
        }
    }

    SECTION("Structured Output Benchmark") {
        // JSON schema for structured output
        json schema = {
            {"type", "object"},
            {"properties", {{"message", {{"type", "string"}}}, {"status", {{"type", "string"}}}}},
            {"required", json::array({"message", "status"})},
            {"additionalProperties", false}};

        // Anthropic - structured responses
        if (anthropicKey) {
            std::vector<std::pair<std::string, Anthropic::Model>> anthropicModels = {
                {"claude-3-5-sonnet-v2", Anthropic::Model::CLAUDE_SONNET_3_5_V2},
                {"claude-opus-4-1", Anthropic::Model::CLAUDE_OPUS_4_1}};

            for (const auto& [shortName, modelEnum] : anthropicModels) {
                try {
                    Anthropic::AnthropicClient client(anthropicKey, modelEnum);

                    LLMRequestConfig config;
                    config.model = Anthropic::toString(modelEnum);
                    config.maxTokens = 100;
                    config.temperature = 0.1f;

                    LLMRequest request(config, structuredPrompt);

                    const auto start = steady_clock::now();
                    auto response = client.sendRequest(request);
                    const auto end = steady_clock::now();

                    const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
                    const double tokensPerSec =
                        elapsedMs > 0 ? (response.usage.totalTokens() * 1000.0) / elapsedMs : 0.0;

                    BenchmarkResult result = {"Anthropic",
                                              shortName,
                                              "structured",
                                              elapsedMs,
                                              response.success,
                                              response.usage.inputTokens,
                                              response.usage.outputTokens,
                                              response.usage.totalTokens(),
                                              tokensPerSec};

                    results.push_back(result);
                    std::cout << result.formatCSV() << std::endl;

                } catch (const std::exception& e) {
                    BenchmarkResult result = {"Anthropic", shortName, "structured", 0, false, 0,
                                              0,           0,         0.0};
                    results.push_back(result);
                    std::cout << result.formatCSV() << " # Error: " << e.what() << std::endl;
                }
            }
        }

        // OpenAI - structured responses using Responses API
        if (openaiKey) {
            std::vector<std::string> openaiModels = {"gpt-4o", "gpt-4o-mini", "gpt-5", "gpt-5-mini",
                                                     "o3-mini"};
            OpenAIClient client(openaiKey);

            for (const auto& modelName : openaiModels) {
                if (isExcludedModel(modelName)) continue;

                try {
                    auto input = OpenAI::ResponsesInput::fromText(structuredPrompt);

                    OpenAI::ResponsesRequest req;
                    req.model = modelName;
                    req.input = input;
                    req.text = OpenAI::TextOutputConfig("bench_structured", schema, true);

                    auto modelEnum = OpenAI::modelFromString(modelName);
                    if (isReasoningModel(modelEnum)) {
                        req.reasoning = json{{"effort", "low"}};
                    }

                    const auto start = steady_clock::now();
                    auto response = client.sendResponsesRequest(req);
                    const auto end = steady_clock::now();

                    const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
                    const int totalTokens =
                        response.usage.inputTokens + response.usage.outputTokens;
                    const double tokensPerSec =
                        elapsedMs > 0 ? (totalTokens * 1000.0) / elapsedMs : 0.0;

                    BenchmarkResult result = {"OpenAI",
                                              modelName,
                                              "structured",
                                              elapsedMs,
                                              response.isCompleted() && !response.hasError(),
                                              response.usage.inputTokens,
                                              response.usage.outputTokens,
                                              totalTokens,
                                              tokensPerSec};

                    results.push_back(result);
                    std::cout << result.formatCSV() << std::endl;

                } catch (const std::exception& e) {
                    BenchmarkResult result = {"OpenAI", modelName, "structured", 0, false, 0,
                                              0,        0,         0.0};
                    results.push_back(result);
                    std::cout << result.formatCSV() << " # Error: " << e.what() << std::endl;
                }
            }
        }
    }

    // Print summary table
    std::cout << "\n=== BENCHMARK SUMMARY ===" << std::endl;
    std::cout << std::left << std::setw(12) << "Provider" << std::setw(25) << "Model"
              << std::setw(12) << "Task" << std::right << std::setw(10) << "Latency" << std::setw(8)
              << "Status" << std::setw(6) << "In" << std::setw(6) << "Out" << std::setw(8)
              << "Tok/s" << std::endl;
    std::cout << std::string(90, '-') << std::endl;

    for (const auto& result : results) {
        std::cout << result.formatTable() << std::endl;
    }

    // Print fastest models by category
    std::cout << "\n=== PERFORMANCE LEADERS ===" << std::endl;

    // Find fastest for simple task
    auto fastestSimple = std::min_element(
        results.begin(), results.end(), [](const BenchmarkResult& a, const BenchmarkResult& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            return a.task == "simple" && b.task == "simple" && a.latencyMs < b.latencyMs;
        });

    if (fastestSimple != results.end() && fastestSimple->task == "simple" &&
        fastestSimple->success) {
        std::cout << "🚀 Fastest Simple Generation: " << fastestSimple->provider << " "
                  << fastestSimple->model << " (" << fastestSimple->latencyMs << "ms)" << std::endl;
    }

    // Find fastest for structured task
    auto fastestStructured = std::min_element(
        results.begin(), results.end(), [](const BenchmarkResult& a, const BenchmarkResult& b) {
            if (!a.success) return false;
            if (!b.success) return true;
            return a.task == "structured" && b.task == "structured" && a.latencyMs < b.latencyMs;
        });

    if (fastestStructured != results.end() && fastestStructured->task == "structured" &&
        fastestStructured->success) {
        std::cout << "📊 Fastest Structured Output: " << fastestStructured->provider << " "
                  << fastestStructured->model << " (" << fastestStructured->latencyMs << "ms)"
                  << std::endl;
    }

    // Basic assertions
    REQUIRE(!results.empty());

    // At least one successful result per provider if keys are available
    if (openaiKey) {
        bool hasOpenAISuccess = std::any_of(
            results.begin(), results.end(),
            [](const BenchmarkResult& r) { return r.provider == "OpenAI" && r.success; });
        REQUIRE(hasOpenAISuccess);
    }

    if (anthropicKey) {
        bool hasAnthropicSuccess = std::any_of(
            results.begin(), results.end(),
            [](const BenchmarkResult& r) { return r.provider == "Anthropic" && r.success; });
        REQUIRE(hasAnthropicSuccess);
    }
}

TEST_CASE("Model Cost Analysis", "[unified][benchmark][cost]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Cost analysis skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    std::cout << "\n=== MODEL COST ANALYSIS ===" << std::endl;
    std::cout
        << "Note: Prices are approximate and may vary. Check provider websites for current pricing."
        << std::endl;

    // Rough pricing per 1K tokens (input/output) in USD - approximate values
    std::map<std::string, std::pair<double, double>> modelPricing = {
        // OpenAI models (input, output per 1K tokens)
        {"gpt-4o", {0.0025, 0.01}},
        {"gpt-4o-mini", {0.00015, 0.0006}},
        {"gpt-5-mini", {0.001, 0.004}},  // Estimated
        {"o1-mini", {0.003, 0.012}},

        // Anthropic models (input, output per 1K tokens)
        {"claude-3-5-haiku", {0.00025, 0.00125}},
        {"claude-3-5-sonnet-v2", {0.003, 0.015}},
        {"claude-opus-4-1", {0.015, 0.075}},  // Estimated
        {"claude-sonnet-4", {0.006, 0.030}}   // Estimated
    };

    std::cout << std::left << std::setw(25) << "Model" << std::right << std::setw(12)
              << "Input $/1K" << std::setw(12) << "Output $/1K" << std::setw(15)
              << "Cost for 10K/1K" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (const auto& [model, pricing] : modelPricing) {
        double inputPrice = pricing.first;
        double outputPrice = pricing.second;
        double costFor10K1K = (10.0 * inputPrice) + (1.0 * outputPrice);  // 10K input, 1K output

        std::cout << std::left << std::setw(25) << model << std::right << std::fixed
                  << std::setprecision(5) << std::setw(12) << inputPrice << std::setw(12)
                  << outputPrice << std::setw(14) << costFor10K1K << "$" << std::endl;
    }

    SUCCEED("Cost analysis completed.");
}
