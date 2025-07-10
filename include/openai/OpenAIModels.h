#pragma once
#include <juce_core/juce_core.h>

namespace OpenAI {
    // Popular OpenAI model names
    namespace Models {
        static const juce::String GPT_4 = "gpt-4";
        static const juce::String GPT_4_TURBO = "gpt-4-turbo";
        static const juce::String GPT_4_TURBO_PREVIEW = "gpt-4-turbo-preview";
        static const juce::String GPT_4_O = "gpt-4o";
        static const juce::String GPT_4_O_MINI = "gpt-4o-mini";
        static const juce::String GPT_3_5_TURBO = "gpt-3.5-turbo";
    }

    // Request structure for OpenAI's Responses API
    template<typename T>
    struct OpenAIResponsesRequest {
        juce::String model = Models::GPT_4_O;
        juce::String previousResponseID{};
        juce::Array<T> inputs;
        
        juce::var toJson() const {
            auto* root = new juce::DynamicObject();
            root->setProperty("model", model);
            
            if (previousResponseID.isNotEmpty()) {
                root->setProperty("previous_response_id", previousResponseID);
            }
            
            juce::Array<juce::var> inputArray;
            for (const auto& input : inputs) {
                inputArray.add(input);
            }
            root->setProperty("inputs", inputArray);
            
            return juce::var(root);
        }
    };

    // Input message structure for OpenAI
    struct InputMessage {
        juce::String role;
        juce::Array<juce::var> content;
        
        juce::var toJson() const {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("role", role);
            obj->setProperty("content", content);
            return juce::var(obj);
        }
    };

    // Input content structure
    struct InputContent {
        juce::String type;
        juce::String text;
        
        juce::var toJson() const {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("type", type);
            obj->setProperty("text", text);
            return juce::var(obj);
        }
    };

    // Function definition for tool calling
    struct FunctionDefinition {
        juce::String name;
        juce::String description;
        juce::var parameters; // JSON schema
        
        juce::var toJson() const {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("name", name);
            obj->setProperty("description", description);
            obj->setProperty("parameters", parameters);
            return juce::var(obj);
        }
    };

    // Tool definition
    struct ToolDefinition {
        juce::String type = "function";
        FunctionDefinition function;
        
        juce::var toJson() const {
            auto* obj = new juce::DynamicObject();
            obj->setProperty("type", type);
            obj->setProperty("function", function.toJson());
            return juce::var(obj);
        }
    };
} 