#pragma once

#include <string>

#include "core/JsonSchemaBuilder.h"
#include "openai/OpenAITypes.h"

/**
 * @brief OpenAI Responses API schema builder
 *
 * Creates schemas compatible with OpenAI's Responses API format.
 * Uses the complex json_schema format with strict mode for structured output.
 *
 * Example usage:
 * @code
 * auto config = OpenAIResponsesSchemaBuilder("sentiment_analysis")
 *     .description("Analyze sentiment of text")
 *     .property("sentiment", JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
 *     .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
 *     .required({"sentiment", "confidence"})
 *     .build();
 * @endcode
 */
class OpenAIResponsesSchemaBuilder {
   public:
    explicit OpenAIResponsesSchemaBuilder(const std::string& functionName);

    // Schema building methods (delegates to JsonSchemaBuilder)
    OpenAIResponsesSchemaBuilder& description(const std::string& desc);
    OpenAIResponsesSchemaBuilder& property(const std::string& name,
                                           const JsonSchemaBuilder& propSchema);
    OpenAIResponsesSchemaBuilder& required(const std::vector<std::string>& requiredProps);
    OpenAIResponsesSchemaBuilder& additionalProperties(bool allowed);

    // OpenAI-specific settings
    OpenAIResponsesSchemaBuilder& strict(bool isStrict = true);

    // Build the final OpenAI TextOutputConfig
    OpenAI::TextOutputConfig build() const;

    // Build just the JSON schema (without OpenAI wrapper)
    json buildSchema() const;

   private:
    std::string functionName_;
    JsonSchemaBuilder schemaBuilder_;
    bool strict_ = true;
};

/**
 * @brief OpenAI Chat Completions API schema builder
 *
 * Creates schemas compatible with OpenAI's Chat Completions API response_format.
 * Uses the simpler format for JSON mode responses.
 *
 * Example usage:
 * @code
 * auto responseFormat = OpenAIChatSchemaBuilder()
 *     .jsonMode()
 *     .build();
 *
 * // Or with schema (for compatible models):
 * auto responseFormat = OpenAIChatSchemaBuilder()
 *     .jsonSchema("user_info", JsonSchemaBuilder::object()
 *         .property("name", JsonSchemaBuilder::string())
 *         .property("age", JsonSchemaBuilder::integer())
 *     )
 *     .build();
 * @endcode
 */
class OpenAIChatSchemaBuilder {
   public:
    OpenAIChatSchemaBuilder() = default;

    // Response format types
    OpenAIChatSchemaBuilder& text();      // Default text response
    OpenAIChatSchemaBuilder& jsonMode();  // JSON mode (basic)
    OpenAIChatSchemaBuilder& jsonSchema(const std::string& name, const JsonSchemaBuilder& schema);

    // Build the final response_format JSON
    json build() const;

   private:
    enum class FormatType { Text, JsonMode, JsonSchema };

    FormatType formatType_ = FormatType::Text;
    std::string schemaName_;
    JsonSchemaBuilder schema_;
};

/**
 * @brief Common schema patterns for OpenAI APIs
 *
 * Provides pre-built schemas for common use cases.
 */
class OpenAISchemaPatterns {
   public:
    // Sentiment analysis schema
    static OpenAI::TextOutputConfig sentimentAnalysis();

    // Data extraction schema
    static OpenAI::TextOutputConfig dataExtraction(const std::vector<std::string>& fields);

    // Classification schema
    static OpenAI::TextOutputConfig classification(const std::vector<std::string>& categories);

    // Summary schema
    static OpenAI::TextOutputConfig summary(int maxLength = 500);

    // Key-value extraction
    static OpenAI::TextOutputConfig keyValueExtraction();

    // Boolean decision
    static OpenAI::TextOutputConfig booleanDecision(const std::string& question);

    // Structured entity extraction
    static OpenAI::TextOutputConfig entityExtraction();

    // Translation schema
    static OpenAI::TextOutputConfig translation(const std::string& targetLanguage);

    // Chat completions patterns
    static json chatJsonMode();
    static json chatClassification(const std::vector<std::string>& categories);
    static json chatSentiment();
};
