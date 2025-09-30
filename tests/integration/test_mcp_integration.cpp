#include <catch2/catch_test_macros.hpp>
#include <openai/OpenAIClient.h>
#include <openai/OpenAIMcpUtils.h>
#include <iostream>

// Helper to check if API key is available
static bool hasApiKey() {
    const char* apiKey = std::getenv("OPENAI_API_KEY");
    return apiKey != nullptr && std::string(apiKey).length() > 0;
}

TEST_CASE("MCP Integration Tests", "[mcp][integration][!shouldfail]") {
    if (!hasApiKey()) {
        SKIP("OPENAI_API_KEY not set - skipping MCP integration tests");
    }

    std::string apiKey = std::getenv("OPENAI_API_KEY");
    OpenAIClient client(apiKey);

    SECTION("MCP tools are configured correctly") {
        // Create a simple config with MCP tools
        LLMRequestConfig config;
        config.client = "OpenAI";
        config.model = "gpt-4o-mini";
        config.temperature = 0.7f;

        // Add MCP tool using the helper function
        OpenAI::McpTool aideasMcpTool;
        aideasMcpTool.serverLabel = "aideas-music-db";
        aideasMcpTool.serverUrl = "https://mcp.musicalaideas.com/mcp";
        aideasMcpTool.requireApproval = "never";

        std::vector<OpenAI::ToolVariant> tools = {aideasMcpTool};
        OpenAI::setTools(config, tools);

        // Verify tools are set
        REQUIRE(OpenAI::hasTools(config));

        json toolsJson = OpenAI::getToolsJson(config);
        REQUIRE(toolsJson.is_array());
        REQUIRE(!toolsJson.empty());

        // Check tool structure
        REQUIRE(toolsJson[0].contains("type"));
        REQUIRE(toolsJson[0]["type"] == "mcp");
        REQUIRE(toolsJson[0].contains("server_label"));
        REQUIRE(toolsJson[0]["server_label"] == "aideas-music-db");
    }

    SECTION("MCP tool call - search by emotional quality") {
        // Create config with MCP tools
        LLMRequestConfig config;
        config.client = "OpenAI";
        config.model = "gpt-4o-mini";
        config.temperature = 0.7f;

        OpenAI::McpTool aideasMcpTool;
        aideasMcpTool.serverLabel = "aideas-music-db";
        aideasMcpTool.serverUrl = "https://mcp.musicalaideas.com/mcp";
        aideasMcpTool.requireApproval = "never";

        std::vector<OpenAI::ToolVariant> tools = {aideasMcpTool};
        OpenAI::setTools(config, tools);

        // Create a request that should trigger MCP tool usage
        std::string prompt =
            "Search the music database for compositions with a happy and energetic feeling.";

        LLMRequest request(config, prompt);

        // Send request
        LLMResponse response = client.sendRequest(request);

        INFO("Response: " << response.toString());

        // Check basic response
        REQUIRE(response.success);
        REQUIRE(!response.responseId.empty());

        // For full MCP analysis, we need the raw ResponsesResponse
        // This test demonstrates the configuration is correct
    }

    SECTION("Parse MCP tool calls from response") {
        // This would require a real API call that returns MCP tool usage
        // For now, we'll create a mock response to test the parsing

        OpenAI::ResponsesResponse mockResponse;
        mockResponse.id = "test-response-123";
        mockResponse.status = OpenAI::ResponseStatus::Completed;

        // Add a mock MCP tool call to output
        json mcpCall = {{"type", "mcp_call"},
                        {"id", "call_123"},
                        {"name", "search_by_emotional_quality"},
                        {"input", {{"quality", "happy"}}},
                        {"output",
                         {{{"title", "Happy Song"}, {"artist", "Joy Band"}, {"key", "C major"}}}}};

        mockResponse.output.push_back(mcpCall);

        // Test extraction
        auto calls = OpenAI::McpUtils::extractMcpCalls(mockResponse);

        REQUIRE(calls.size() == 1);
        REQUIRE(calls[0].id == "call_123");
        REQUIRE(calls[0].tool == "search_by_emotional_quality");
        REQUIRE(calls[0].success);
        REQUIRE(calls[0].output.has_value());

        // Test helper functions
        REQUIRE(OpenAI::McpUtils::wasToolCalled(mockResponse, "search_by_emotional_quality"));
        REQUIRE(!OpenAI::McpUtils::wasToolCalled(mockResponse, "nonexistent_tool"));

        auto output = OpenAI::McpUtils::getToolOutput(mockResponse, "search_by_emotional_quality");
        REQUIRE(output.has_value());
        REQUIRE(output.value().is_array());

        // Test stats
        json stats = OpenAI::McpUtils::getMcpUsageStats(mockResponse);
        REQUIRE(stats["total_calls"] == 1);
        REQUIRE(stats["successful_calls"] == 1);
        REQUIRE(stats["failed_calls"] == 0);
        REQUIRE(stats["tools_used"].size() == 1);
    }

    SECTION("Parse multiple MCP tool calls") {
        OpenAI::ResponsesResponse mockResponse;
        mockResponse.id = "test-response-456";
        mockResponse.status = OpenAI::ResponseStatus::Completed;

        // Add mcp_list_tools
        json listTools = {{"type", "mcp_list_tools"},
                          {"tools",
                           {{"name", "search_music"}, {"name", "search_by_artist"}, {"name", "get_composition"}}}};

        mockResponse.output.push_back(listTools);

        // Add multiple tool calls
        json call1 = {{"type", "mcp_call"},
                      {"id", "call_1"},
                      {"name", "search_by_key_mode"},
                      {"input", {{"key", "C"}, {"mode", "major"}}},
                      {"output", {{"count", 100}}}};

        json call2 = {{"type", "mcp_call"},
                      {"id", "call_2"},
                      {"name", "search_by_artist"},
                      {"input", {{"artist", "Miles Davis"}}},
                      {"output", {{"count", 42}}}};

        mockResponse.output.push_back(call1);
        mockResponse.output.push_back(call2);

        // Test tools listing
        REQUIRE(OpenAI::McpUtils::wereMcpToolsListed(mockResponse));

        auto availableTools = OpenAI::McpUtils::getAvailableMcpTools(mockResponse);
        REQUIRE(availableTools.size() == 3);
        REQUIRE(std::find(availableTools.begin(), availableTools.end(), "search_music") !=
                availableTools.end());

        // Test multiple calls
        auto calls = OpenAI::McpUtils::extractMcpCalls(mockResponse);
        REQUIRE(calls.size() == 2);

        // Test wereAllToolsCalled
        std::vector<std::string> expectedTools = {"search_by_key_mode", "search_by_artist"};
        REQUIRE(OpenAI::McpUtils::wereAllToolsCalled(mockResponse, expectedTools));

        std::vector<std::string> notAllCalled = {"search_by_key_mode", "nonexistent"};
        REQUIRE(!OpenAI::McpUtils::wereAllToolsCalled(mockResponse, notAllCalled));

        // Test stats
        json stats = OpenAI::McpUtils::getMcpUsageStats(mockResponse);
        REQUIRE(stats["total_calls"] == 2);
        REQUIRE(stats["successful_calls"] == 2);
        REQUIRE(stats["tools_used"].size() == 2);
    }

    SECTION("Handle MCP tool call errors") {
        OpenAI::ResponsesResponse mockResponse;
        mockResponse.id = "test-response-error";
        mockResponse.status = OpenAI::ResponseStatus::Completed;

        // Add a failed tool call
        json failedCall = {{"type", "mcp_call"},
                           {"id", "call_fail"},
                           {"name", "search_music"},
                           {"input", {{"query", "test"}}},
                           {"error", "Database connection failed"}};

        mockResponse.output.push_back(failedCall);

        auto calls = OpenAI::McpUtils::extractMcpCalls(mockResponse);
        REQUIRE(calls.size() == 1);
        REQUIRE(!calls[0].success);
        REQUIRE(calls[0].error.has_value());
        REQUIRE(calls[0].error.value() == "Database connection failed");

        // Stats should show failed call
        json stats = OpenAI::McpUtils::getMcpUsageStats(mockResponse);
        REQUIRE(stats["total_calls"] == 1);
        REQUIRE(stats["successful_calls"] == 0);
        REQUIRE(stats["failed_calls"] == 1);
    }
}
