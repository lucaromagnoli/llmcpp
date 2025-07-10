#include <doctest/doctest.h>
#include "openai/OpenAITypes.h"

using namespace OpenAI;

TEST_CASE("OpenAI::TextInput serialization") {
    TextInput input;
    input.text = "Hello, world!";
    
    json j = input.toJson();
    
    REQUIRE(j["text"] == "Hello, world!");
    REQUIRE(j["type"] == "input_text");
}

TEST_CASE("OpenAI::ImageInput serialization") {
    ImageInput input;
    input.detail = "high";
    input.fileId = "file-123";
    input.imageUrl = "https://example.com/image.jpg";
    
    json j = input.toJson();
    
    REQUIRE(j["detail"] == "high");
    REQUIRE(j["type"] == "input_image");
    REQUIRE(j["file_id"] == "file-123");
    REQUIRE(j["image_url"] == "https://example.com/image.jpg");
}

TEST_CASE("OpenAI::FileInput serialization") {
    FileInput input;
    input.fileId = "file-456";
    input.filename = "document.pdf";
    
    json j = input.toJson();
    
    REQUIRE(j["type"] == "input_file");
    REQUIRE(j["file_id"] == "file-456");
    REQUIRE(j["filename"] == "document.pdf");
    REQUIRE(!j.contains("file_data"));
}

TEST_CASE("OpenAI::InputMessage with string content") {
    InputMessage message;
    message.role = InputMessage::Role::User;
    message.content = "What is the weather like?";
    
    json j = message.toJson();
    
    REQUIRE(j["role"] == "user");
    REQUIRE(j["content"] == "What is the weather like?");
}

TEST_CASE("OpenAI::InputMessage with content array", "[openai][types]") {
    InputMessage message;
    message.role = InputMessage::Role::System;
    
    std::vector<InputContent> content;
    TextInput textInput;
    textInput.text = "System instructions";
    content.push_back(textInput);
    
    ImageInput imageInput;
    imageInput.imageUrl = "https://example.com/chart.png";
    content.push_back(imageInput);
    
    message.content = content;
    
    json j = message.toJson();
    
    REQUIRE(j["role"] == "system");
    REQUIRE(j["content"].is_array());
    REQUIRE(j["content"].size() == 2);
    REQUIRE(j["content"][0]["type"] == "input_text");
    REQUIRE(j["content"][1]["type"] == "input_image");
}

TEST_CASE("OpenAI::InputMessage role conversion", "[openai][types]") {
    REQUIRE(InputMessage::roleToString(InputMessage::Role::User) == "user");
    REQUIRE(InputMessage::roleToString(InputMessage::Role::Assistant) == "assistant");
    REQUIRE(InputMessage::roleToString(InputMessage::Role::System) == "system");
    REQUIRE(InputMessage::roleToString(InputMessage::Role::Developer) == "developer");
    
    REQUIRE(InputMessage::stringToRole("user") == InputMessage::Role::User);
    REQUIRE(InputMessage::stringToRole("assistant") == InputMessage::Role::Assistant);
    REQUIRE(InputMessage::stringToRole("system") == InputMessage::Role::System);
    REQUIRE(InputMessage::stringToRole("developer") == InputMessage::Role::Developer);
    
    REQUIRE_THROWS_AS(InputMessage::stringToRole("invalid"), std::invalid_argument);
}

TEST_CASE("OpenAI::ResponsesInput from text", "[openai][types]") {
    auto input = ResponsesInput::fromText("Simple text input");
    
    REQUIRE(input.type == ResponsesInput::Type::String);
    REQUIRE(input.textInput == "Simple text input");
    REQUIRE(input.contentList.empty());
    
    json j = input.toJson();
    REQUIRE(j.is_string());
    REQUIRE(j == "Simple text input");
}

TEST_CASE("OpenAI::ResponsesInput from content list", "[openai][types]") {
    std::vector<InputMessage> messages;
    
    InputMessage userMsg;
    userMsg.role = InputMessage::Role::User;
    userMsg.content = "Hello";
    messages.push_back(userMsg);
    
    InputMessage systemMsg;
    systemMsg.role = InputMessage::Role::System;
    systemMsg.content = "Instructions";
    messages.push_back(systemMsg);
    
    auto input = ResponsesInput::fromContentList(messages);
    
    REQUIRE(input.type == ResponsesInput::Type::ContentList);
    REQUIRE(input.contentList.size() == 2);
    
    json j = input.toJson();
    REQUIRE(j.is_array());
    REQUIRE(j.size() == 2);
    REQUIRE(j[0]["role"] == "user");
    REQUIRE(j[1]["role"] == "system");
}

TEST_CASE("OpenAI::TextOutputConfig serialization", "[openai][types]") {
    json schema = json::parse(R"({
        "type": "object",
        "properties": {
            "result": {"type": "string"}
        },
        "required": ["result"]
    })");
    
    TextOutputConfig config("test_function", schema, true);
    
    json j = config.toJson();
    
    REQUIRE(j.contains("format"));
    REQUIRE(j["format"]["type"] == "json_schema");
    REQUIRE(j["format"]["name"] == "test_function");
    REQUIRE(j["format"]["strict"] == true);
    REQUIRE(j["format"]["schema"] == schema);
}

TEST_CASE("OpenAI::FunctionTool serialization", "[openai][types]") {
    FunctionTool tool;
    tool.name = "get_weather";
    tool.description = "Get weather information";
    tool.parameters = json::parse(R"({
        "type": "object",
        "properties": {
            "location": {"type": "string"}
        }
    })");
    
    json j = tool.toJson();
    
    REQUIRE(j["name"] == "get_weather");
    REQUIRE(j["type"] == "function");
    REQUIRE(j["strict"] == true);
    REQUIRE(j["description"] == "Get weather information");
    REQUIRE(j["parameters"]["type"] == "object");
}

TEST_CASE("OpenAI::WebSearchTool serialization", "[openai][types]") {
    WebSearchTool tool;
    tool.searchContextSize = "high";
    
    json j = tool.toJson();
    
    REQUIRE(j["type"] == "web_search");
    REQUIRE(j["search_context_size"] == "high");
}

TEST_CASE("OpenAI::FileSearchTool serialization", "[openai][types]") {
    FileSearchTool tool;
    tool.vectorStoreIds = {"store1", "store2"};
    tool.maxNumResults = 10;
    
    json j = tool.toJson();
    
    REQUIRE(j["type"] == "file_search");
    REQUIRE(j["vector_store_ids"].is_array());
    REQUIRE(j["vector_store_ids"].size() == 2);
    REQUIRE(j["max_num_results"] == 10);
}

TEST_CASE("OpenAI::ToolChoiceMode conversion", "[openai][types]") {
    REQUIRE(toString(ToolChoiceMode::None) == "none");
    REQUIRE(toString(ToolChoiceMode::Auto) == "auto");
    REQUIRE(toString(ToolChoiceMode::Required) == "required");
}

TEST_CASE("OpenAI::ResponsesRequest basic serialization", "[openai][types]") {
    ResponsesRequest request;
    request.model = "gpt-4o";
    request.instructions = "Answer the question";
    request.input = ResponsesInput::fromText("What is 2+2?");
    
    json schema = json::parse(R"({
        "type": "object",
        "properties": {
            "answer": {"type": "number"}
        }
    })");
    request.text = TextOutputConfig("math_answer", schema);
    
    json j = request.toJson();
    
    REQUIRE(j["model"] == "gpt-4o");
    REQUIRE(j["instructions"] == "Answer the question");
    REQUIRE(j["input"] == "What is 2+2?");
    REQUIRE(j["tool_choice"] == "auto");
    REQUIRE(j["text"]["format"]["name"] == "math_answer");
}

TEST_CASE("OpenAI::ResponsesRequest with tools", "[openai][types]") {
    ResponsesRequest request;
    request.model = "gpt-4o";
    request.instructions = "Use tools to help";
    request.input = ResponsesInput::fromText("Get the weather");
    
    FunctionTool weatherTool;
    weatherTool.name = "get_weather";
    weatherTool.parameters = json::parse(R"({"type": "object", "properties": {"location": {"type": "string"}}})");
    
    WebSearchTool searchTool;
    searchTool.searchContextSize = "medium";
    
    request.tools = std::vector<ToolVariant>{weatherTool, searchTool};
    
    json j = request.toJson();
    
    REQUIRE(j.contains("tools"));
    REQUIRE(j["tools"].is_array());
    REQUIRE(j["tools"].size() == 2);
    REQUIRE(j["tools"][0]["type"] == "function");
    REQUIRE(j["tools"][1]["type"] == "web_search");
}

TEST_CASE("OpenAI::ChatMessage serialization", "[openai][types]") {
    ChatMessage message;
    message.role = "user";
    message.content = "Hello, how are you?";
    message.name = "Alice";
    
    json j = message.toJson();
    
    REQUIRE(j["role"] == "user");
    REQUIRE(j["content"] == "Hello, how are you?");
    REQUIRE(j["name"] == "Alice");
}

TEST_CASE("OpenAI::ChatMessage deserialization", "[openai][types]") {
    json j = json::parse(R"({
        "role": "assistant",
        "content": "I'm doing well, thank you!",
        "name": "Bot"
    })");
    
    ChatMessage message = ChatMessage::fromJson(j);
    
    REQUIRE(message.role == "assistant");
    REQUIRE(message.content == "I'm doing well, thank you!");
    REQUIRE(message.name == "Bot");
}

TEST_CASE("OpenAI::ChatCompletionRequest serialization", "[openai][types]") {
    ChatCompletionRequest request;
    request.model = "gpt-4";
    request.temperature = 0.7;
    request.maxTokens = 100;
    
    ChatMessage systemMsg;
    systemMsg.role = "system";
    systemMsg.content = "You are a helpful assistant.";
    
    ChatMessage userMsg;
    userMsg.role = "user";
    userMsg.content = "Hello!";
    
    request.messages = {systemMsg, userMsg};
    
    json j = request.toJson();
    
    REQUIRE(j["model"] == "gpt-4");
    REQUIRE(j["temperature"] == Catch::Approx(0.7));
    REQUIRE(j["max_tokens"] == 100);
    REQUIRE(j["messages"].is_array());
    REQUIRE(j["messages"].size() == 2);
    REQUIRE(j["messages"][0]["role"] == "system");
    REQUIRE(j["messages"][1]["role"] == "user");
}

TEST_CASE("OpenAI::ChatCompletionChoice deserialization", "[openai][types]") {
    json j = json::parse(R"({
        "index": 0,
        "message": {
            "role": "assistant",
            "content": "Hello! How can I help you?"
        },
        "finish_reason": "stop"
    })");
    
    ChatCompletionChoice choice = ChatCompletionChoice::fromJson(j);
    
    REQUIRE(choice.index == 0);
    REQUIRE(choice.message.role == "assistant");
    REQUIRE(choice.message.content == "Hello! How can I help you?");
    REQUIRE(choice.finishReason == "stop");
}

TEST_CASE("OpenAI::CompletionRequest serialization", "[openai][types]") {
    CompletionRequest request;
    request.model = "text-davinci-003";
    request.prompt = "Once upon a time";
    request.maxTokens = 50;
    request.temperature = 0.8;
    request.stop = std::vector<std::string>{"\n", "."};
    
    json j = request.toJson();
    
    REQUIRE(j["model"] == "text-davinci-003");
    REQUIRE(j["prompt"] == "Once upon a time");
    REQUIRE(j["max_tokens"] == 50);
    REQUIRE(j["temperature"] == Catch::Approx(0.8));
    REQUIRE(j["stop"].is_array());
    REQUIRE(j["stop"].size() == 2);
}

TEST_CASE("OpenAI::CompletionChoice deserialization", "[openai][types]") {
    json j = json::parse(R"({
        "text": ", there was a kingdom",
        "index": 0,
        "finish_reason": "length"
    })");
    
    CompletionChoice choice = CompletionChoice::fromJson(j);
    
    REQUIRE(choice.text == ", there was a kingdom");
    REQUIRE(choice.index == 0);
    REQUIRE(choice.finishReason == "length");
}

TEST_CASE("OpenAI::OpenAIConfig serialization", "[openai][types]") {
    OpenAIConfig config;
    config.apiKey = "sk-test123";
    config.organization = "org-test";
    config.project = "proj-test";
    config.timeoutSeconds = 60;
    config.maxRetries = 5;
    config.enableDeprecationWarnings = false;
    
    json j = config.toJson();
    
    REQUIRE(j["api_key"] == "sk-test123");
    REQUIRE(j["organization"] == "org-test");
    REQUIRE(j["project"] == "proj-test");
    REQUIRE(j["timeout_seconds"] == 60);
    REQUIRE(j["max_retries"] == 5);
    REQUIRE(j["enable_deprecation_warnings"] == false);
}

TEST_CASE("OpenAI::OpenAIConfig deserialization", "[openai][types]") {
    json j = json::parse(R"({
        "api_key": "sk-test456",
        "base_url": "https://api.openai.com/v1",
        "organization": "org-test2",
        "timeout_seconds": 45,
        "enable_deprecation_warnings": true
    })");
    
    OpenAIConfig config = OpenAIConfig::fromJson(j);
    
    REQUIRE(config.apiKey == "sk-test456");
    REQUIRE(config.baseUrl == "https://api.openai.com/v1");
    REQUIRE(config.organization == "org-test2");
    REQUIRE(config.timeoutSeconds == 45);
    REQUIRE(config.enableDeprecationWarnings == true);
    REQUIRE(config.maxRetries == 3);  // Default value
}

TEST_CASE("OpenAI::OpenAIConfig default values", "[openai][types]") {
    OpenAIConfig config;
    
    REQUIRE(config.baseUrl == "https://api.openai.com/v1");
    REQUIRE(config.timeoutSeconds == 30);
    REQUIRE(config.maxRetries == 3);
    REQUIRE(config.enableDeprecationWarnings == true);
}

TEST_CASE("OpenAI::ApiType enum values", "[openai][types]") {
    // Test that enum values are properly defined
    ApiType responses = ApiType::RESPONSES;
    ApiType chat = ApiType::CHAT_COMPLETIONS;
    ApiType completions = ApiType::COMPLETIONS;
    ApiType autoDetect = ApiType::AUTO_DETECT;
    
    REQUIRE(responses != chat);
    REQUIRE(chat != completions);
    REQUIRE(completions != autoDetect);
}

// Integration test combining multiple types
TEST_CASE("OpenAI::ResponsesRequest complex integration", "[openai][types]") {
    // Create a complex request with multiple input types
    std::vector<InputMessage> messages;
    
    // System message with text input
    InputMessage systemMsg;
    systemMsg.role = InputMessage::Role::System;
    systemMsg.content = "You are a helpful assistant that analyzes images and text.";
    messages.push_back(systemMsg);
    
    // User message with mixed content
    InputMessage userMsg;
    userMsg.role = InputMessage::Role::User;
    
    std::vector<InputContent> userContent;
    TextInput textInput;
    textInput.text = "Please analyze this image:";
    userContent.push_back(textInput);
    
    ImageInput imageInput;
    imageInput.imageUrl = "https://example.com/chart.png";
    imageInput.detail = "high";
    userContent.push_back(imageInput);
    
    userMsg.content = userContent;
    messages.push_back(userMsg);
    
    // Create the request
    ResponsesRequest request;
    request.model = "gpt-4o";
    request.input = ResponsesInput::fromContentList(messages);
    request.instructions = "Analyze the image and provide insights";
    request.temperature = 0.3;
    request.maxOutputTokens = 500;
    
    // Add structured output schema
    json schema = json::parse(R"({
        "type": "object",
        "properties": {
            "analysis": {"type": "string"},
            "confidence": {"type": "number", "minimum": 0, "maximum": 1},
            "insights": {
                "type": "array",
                "items": {"type": "string"}
            }
        },
        "required": ["analysis", "confidence", "insights"]
    })");
    request.text = TextOutputConfig("image_analysis", schema);
    
    // Add function tool
    FunctionTool searchTool;
    searchTool.name = "search_additional_info";
    searchTool.description = "Search for additional information";
    searchTool.parameters = json::parse(R"({
        "type": "object",
        "properties": {
            "query": {"type": "string"}
        }
    })");
    
    request.tools = std::vector<ToolVariant>{searchTool};
    request.toolChoice = ToolChoiceMode::Auto;
    
    // Test serialization
    json j = request.toJson();
    
    // Verify structure
    REQUIRE(j["model"] == "gpt-4o");
    REQUIRE(j["instructions"] == "Analyze the image and provide insights");
    REQUIRE(j["temperature"] == Catch::Approx(0.3));
    REQUIRE(j["max_output_tokens"] == 500);
    REQUIRE(j["tool_choice"] == "auto");
    
    // Verify input structure
    REQUIRE(j["input"].is_array());
    REQUIRE(j["input"].size() == 2);
    REQUIRE(j["input"][0]["role"] == "system");
    REQUIRE(j["input"][1]["role"] == "user");
    REQUIRE(j["input"][1]["content"].is_array());
    REQUIRE(j["input"][1]["content"].size() == 2);
    REQUIRE(j["input"][1]["content"][0]["type"] == "input_text");
    REQUIRE(j["input"][1]["content"][1]["type"] == "input_image");
    
    // Verify output schema
    REQUIRE(j["text"]["format"]["type"] == "json_schema");
    REQUIRE(j["text"]["format"]["name"] == "image_analysis");
    REQUIRE(j["text"]["format"]["schema"]["type"] == "object");
    
    // Verify tools
    REQUIRE(j["tools"].is_array());
    REQUIRE(j["tools"].size() == 1);
    REQUIRE(j["tools"][0]["type"] == "function");
    REQUIRE(j["tools"][0]["name"] == "search_additional_info");
}

// Test error handling
TEST_CASE("OpenAI::Error handling in type conversions", "[openai][types]") {
    // Test invalid role conversion
    REQUIRE_THROWS_AS(InputMessage::stringToRole("invalid_role"), std::invalid_argument);
    
    // Test ChatMessage with incomplete JSON
    json incompleteJson = json::parse(R"({"role": "user"})");
    REQUIRE_THROWS_AS(ChatMessage::fromJson(incompleteJson), json::exception);
    
    // Test CompletionChoice with incomplete JSON
    json incompleteChoice = json::parse(R"({"index": 0})");
    REQUIRE_THROWS_AS(CompletionChoice::fromJson(incompleteChoice), json::exception);
}

// Test JSON roundtrip serialization
TEST_CASE("OpenAI::JSON roundtrip consistency", "[openai][types]") {
    // Test ChatMessage roundtrip
    ChatMessage original;
    original.role = "assistant";
    original.content = "Hello, world!";
    original.name = "TestBot";
    
    json j = original.toJson();
    ChatMessage deserialized = ChatMessage::fromJson(j);
    
    REQUIRE(original.role == deserialized.role);
    REQUIRE(original.content == deserialized.content);
    REQUIRE(original.name == deserialized.name);
    
    // Test OpenAIConfig roundtrip
    OpenAIConfig originalConfig;
    originalConfig.apiKey = "test-key";
    originalConfig.organization = "test-org";
    originalConfig.timeoutSeconds = 45;
    
    json configJson = originalConfig.toJson();
    OpenAIConfig deserializedConfig = OpenAIConfig::fromJson(configJson);
    
    REQUIRE(originalConfig.apiKey == deserializedConfig.apiKey);
    REQUIRE(originalConfig.organization == deserializedConfig.organization);
    REQUIRE(originalConfig.timeoutSeconds == deserializedConfig.timeoutSeconds);
} 