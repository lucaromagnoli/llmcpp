#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "openai/OpenAITypes.h"

using namespace OpenAI;

// Simple helper to build a representative ResponsesRequest
static ResponsesRequest makeRequest() {
    ResponsesRequest r;
    r.model = "gpt-4o";
    r.instructions = "Answer briefly.";
    r.maxOutputTokens = 128;
    r.toolChoice = ToolChoiceMode::Auto;

    // Add a small JSON schema
    json schema = json::parse(R"({
        "type":"object",
        "properties":{"answer":{"type":"string"}},
        "required":["answer"]
    })");
    r.text = TextOutputConfig("answer_schema", schema, true);

    // Add a couple of input messages
    std::vector<InputMessage> messages;
    InputMessage sys;
    sys.role = InputMessage::Role::System;
    sys.content = "You are helpful.";
    messages.push_back(sys);

    InputMessage usr;
    usr.role = InputMessage::Role::User;
    usr.content = "Hello";
    messages.push_back(usr);

    r.input = ResponsesInput::fromContentList(messages);
    return r;
}

TEST_CASE("Benchmark: ResponsesRequest serialization", "[benchmark]") {
    auto req = makeRequest();
    BENCHMARK("toJson serialize") {
        return req.toJson();
    };
}

TEST_CASE("Benchmark: ResponsesResponse parsing", "[benchmark]") {
    // Minimal example response JSON
    auto sample = json::parse(R"({
        "id": "resp_123",
        "object": "response",
        "created_at": 0,
        "status": "completed",
        "model": "gpt-4o-mini-2024-07-18",
        "usage": {"input_tokens": 10, "output_tokens": 5},
        "output": [
          {"type":"message","id":"msg_1","role":"assistant","content":[{"type":"output_text","text":"Hi"}]}
        ]
    })");

    BENCHMARK("fromJson parse") {
        return ResponsesResponse::fromJson(sample);
    };
}

TEST_CASE("Benchmark: Model enum conversions", "[benchmark]") {
    BENCHMARK("modelToString") {
        return toString(Model::GPT_4o);
    };
    BENCHMARK("stringToModel") {
        return modelFromString("gpt-4o");
    };
}


