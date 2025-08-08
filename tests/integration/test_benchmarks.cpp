#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>

#include "openai/OpenAIClient.h"
#include "openai/OpenAITypes.h"

using namespace std::chrono;

static bool isReasoningModel(OpenAI::Model model) {
    return model == OpenAI::Model::GPT_5 || model == OpenAI::Model::GPT_5_Mini ||
           model == OpenAI::Model::GPT_5_Nano || model == OpenAI::Model::O3 ||
           model == OpenAI::Model::O3_Mini || model == OpenAI::Model::O1 ||
           model == OpenAI::Model::O1_Mini || model == OpenAI::Model::O1_Preview ||
           model == OpenAI::Model::O1_Pro || model == OpenAI::Model::O4_Mini;
}

TEST_CASE("OpenAI model benchmarks (structured outputs)", "[openai][integration][benchmark]") {
    const char* runBenchEnv = std::getenv("LLMCPP_RUN_BENCHMARKS");
    if (!runBenchEnv || std::string(runBenchEnv) != "1") {
        SUCCEED("Benchmarks skipped. Set LLMCPP_RUN_BENCHMARKS=1 to enable.");
        return;
    }

    const char* apiKey = std::getenv("OPENAI_API_KEY");
    REQUIRE(apiKey != nullptr);

    OpenAIClient client(apiKey);

    // Minimal structured output schema
    json schema = {{"type", "object"},
                   {"properties", {{"answer", {{"type", "string"}}}}},
                   {"required", json::array({"answer"})}};

    // Simple input
    auto input = OpenAI::ResponsesInput::fromText("Reply with the word OK.");

    // Iterate through response-capable models
    for (const auto& modelName : OpenAI::RESPONSES_MODELS) {
        DYNAMIC_SECTION("Benchmark model: " << modelName) {
            OpenAI::ResponsesRequest req;
            req.model = modelName;
            req.input = input;
            req.text = OpenAI::TextOutputConfig("bench_schema", schema, true);
            req.maxOutputTokens = 16;

            // Tweak reasoning parameters when appropriate
            auto modelEnum = OpenAI::modelFromString(modelName);
            if (isReasoningModel(modelEnum)) {
                req.reasoningEffort = std::string("low");
            }

            const auto start = steady_clock::now();
            auto response = client.sendResponsesRequest(req);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
            std::cout << "[BENCH] model=" << modelName << ", ms=" << elapsedMs
                      << ", success=" << (response.isCompleted() && !response.hasError())
                      << std::endl;

            // Sanity: we should at least get a response object back; don't assert success to avoid
            // flakes
            REQUIRE(!response.id.empty());
        }
    }
}
