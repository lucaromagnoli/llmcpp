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

static bool isExcludedModel(const std::string &modelName) {
    return modelName == "gpt-image-1" || modelName == "computer-use-preview";
}

static bool isGpt5Family(OpenAI::Model model) {
    return model == OpenAI::Model::GPT_5 || model == OpenAI::Model::GPT_5_Mini ||
           model == OpenAI::Model::GPT_5_Nano;
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
                   {"required", json::array({"answer"})},
                   {"additionalProperties", false}};

    // Simple input aligned with structured output requirement
    auto input = OpenAI::ResponsesInput::fromText(
        "Return a JSON object that conforms to the provided schema with answer set to 'OK'.");

    // Iterate through response-capable models
    for (const auto& modelName : OpenAI::RESPONSES_MODELS) {
        if (isExcludedModel(modelName)) {
            std::cout << "[BENCH] skipping model=" << modelName << " (not supported for JSON schema bench)" << std::endl;
            continue;
        }
        DYNAMIC_SECTION("Benchmark model: " << modelName) {
            OpenAI::ResponsesRequest req;
            req.model = modelName;
            req.input = input;
            req.text = OpenAI::TextOutputConfig("bench_schema", schema, true);
            // Token cap logic:
            // - For GPT-5 family: do NOT set max_output_tokens (let server decide)
            // - For reasoning O-series: use a higher cap (128)
            // - For others: small cap (16) to focus on latency
            auto modelEnum = OpenAI::modelFromString(modelName);
            if (!isGpt5Family(modelEnum)) {
                int cap = isReasoningModel(modelEnum) ? 128 : 16;
                req.maxOutputTokens = cap;
            }

            // Tweak reasoning parameters when appropriate
            if (isReasoningModel(modelEnum)) {
                req.reasoning = json{{"effort", "low"}};
            }

            const auto start = steady_clock::now();
            auto response = client.sendResponsesRequest(req);
            const auto end = steady_clock::now();

            const auto elapsedMs = duration_cast<milliseconds>(end - start).count();
            const bool ok = (response.isCompleted() && !response.hasError());
            std::cout << modelName << "," << elapsedMs << "," << (ok ? "ok" : "fail") << std::endl;

            // Sanity: we should at least get a response object back; don't assert success to avoid
            // flakes
            REQUIRE(!response.id.empty());
        }
    }
}
