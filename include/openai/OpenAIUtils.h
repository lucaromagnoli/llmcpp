#pragma once
#include <juce_core/juce_core.h>

namespace OpenAIUtils {
    /**
     * Get the output data from OpenAI's Responses API
     */
    inline juce::DynamicObject::Ptr parseResponsesOutput(const juce::var& result) {
        const auto* obj = result.getDynamicObject();
        if (obj == nullptr) {
            DBG("Invalid response format: Not a valid JSON object");
            return {};
        }

        const auto output = obj->getProperty("output");
        if (output.isArray() && output.size() > 0) {
            if (const auto* firstElement = output[0].getDynamicObject();
                firstElement != nullptr && firstElement->hasProperty("content")) {
                const auto& contentArray = firstElement->getProperty("content");
                if (contentArray.isArray() && contentArray.size() > 0) {
                    if (const auto* firstContent = contentArray[0].getDynamicObject();
                        firstContent != nullptr && firstContent->hasProperty("text")) {
                        if (const auto& text = firstContent->getProperty("text"); text.isString()) {
                            return juce::JSON::parse(text.toString()).getDynamicObject();
                        }
                        DBG("Invalid response format: 'text' is not a string");
                        return {};
                    }
                    DBG("Invalid response format: 'content' does not contain 'text'");
                    return {};
                }
            }
        }

        return obj->clone().get(); // fallback: return the whole original object
    }

    /**
     * Parse metadata from OpenAI's Responses API
     */
    inline juce::DynamicObject::Ptr parseResponsesMetadata(const juce::var& result) {
        const auto* obj = result.getDynamicObject();
        if (obj == nullptr)
            return {};

        auto metadata = new juce::DynamicObject();

        if (obj->hasProperty("usage"))
            metadata->setProperty("usage", obj->getProperty("usage"));

        if (obj->hasProperty("model"))
            metadata->setProperty("model", obj->getProperty("model"));

        if (obj->hasProperty("user"))
            metadata->setProperty("user", obj->getProperty("user"));

        if (obj->hasProperty("id"))
            metadata->setProperty("id", obj->getProperty("id"));

        return metadata;
    }

    /**
     * Build HTTP headers for OpenAI API requests
     */
    inline juce::String buildHeaders(const juce::String& apiKey) {
        juce::String headers;
        headers << "Content-Type: application/json\r\n";
        headers << "Authorization: Bearer " + apiKey;
        return headers;
    }

    /**
     * Convert StringPairArray to debug string
     */
    inline juce::String headersToString(const juce::StringPairArray& headers) {
        juce::String result;
        for (int i = 0; i < headers.size(); ++i) {
            result += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\n";
        }
        return result;
    }

    /**
     * Build OpenAI API URL with POST data
     */
    inline juce::URL buildURLWithPostData(const juce::String& payload, bool useResponsesAPI) {
        const juce::String endpoint = useResponsesAPI ? "v1/responses" : "v1/chat/completions";
        const juce::URL url("https://api.openai.com/" + endpoint);
        return url.withPOSTData(payload);
    }
} 