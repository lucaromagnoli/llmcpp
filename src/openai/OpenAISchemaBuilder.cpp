#include "openai/OpenAISchemaBuilder.h"

// OpenAIResponsesSchemaBuilder implementation
OpenAIResponsesSchemaBuilder::OpenAIResponsesSchemaBuilder(const std::string& functionName)
    : functionName_(functionName) {
    schemaBuilder_.type("object");  // Responses API schemas are always objects
}

OpenAIResponsesSchemaBuilder& OpenAIResponsesSchemaBuilder::description(const std::string& desc) {
    schemaBuilder_.description(desc);
    return *this;
}

OpenAIResponsesSchemaBuilder& OpenAIResponsesSchemaBuilder::property(
    const std::string& name, const JsonSchemaBuilder& propSchema) {
    schemaBuilder_.property(name, propSchema);
    return *this;
}

OpenAIResponsesSchemaBuilder& OpenAIResponsesSchemaBuilder::required(
    const std::vector<std::string>& requiredProps) {
    schemaBuilder_.required(requiredProps);
    return *this;
}

OpenAIResponsesSchemaBuilder& OpenAIResponsesSchemaBuilder::additionalProperties(bool allowed) {
    schemaBuilder_.additionalProperties(allowed);
    return *this;
}

OpenAIResponsesSchemaBuilder& OpenAIResponsesSchemaBuilder::strict(bool isStrict) {
    strict_ = isStrict;
    return *this;
}

OpenAI::TextOutputConfig OpenAIResponsesSchemaBuilder::build() const {
    return OpenAI::TextOutputConfig(functionName_, schemaBuilder_.build(), strict_);
}

json OpenAIResponsesSchemaBuilder::buildSchema() const { return schemaBuilder_.build(); }

// OpenAIChatSchemaBuilder implementation
OpenAIChatSchemaBuilder& OpenAIChatSchemaBuilder::text() {
    formatType_ = FormatType::Text;
    return *this;
}

OpenAIChatSchemaBuilder& OpenAIChatSchemaBuilder::jsonMode() {
    formatType_ = FormatType::JsonMode;
    return *this;
}

OpenAIChatSchemaBuilder& OpenAIChatSchemaBuilder::jsonSchema(const std::string& name,
                                                             const JsonSchemaBuilder& schema) {
    formatType_ = FormatType::JsonSchema;
    schemaName_ = name;
    schema_ = schema;
    return *this;
}

json OpenAIChatSchemaBuilder::build() const {
    switch (formatType_) {
        case FormatType::Text:
            return json{{"type", "text"}};

        case FormatType::JsonMode:
            return json{{"type", "json_object"}};

        case FormatType::JsonSchema:
            return json{{"type", "json_schema"},
                        {"json_schema",
                         {{"name", schemaName_}, {"schema", schema_.build()}, {"strict", true}}}};
    }
    return json{{"type", "text"}};  // fallback
}

// OpenAISchemaPatterns implementation
OpenAI::TextOutputConfig OpenAISchemaPatterns::sentimentAnalysis() {
    return OpenAIResponsesSchemaBuilder("sentiment_analysis")
        .description("Analyze the sentiment of the given text")
        .property("sentiment", JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
        .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1).description(
                                    "Confidence score from 0 to 1"))
        .required({"sentiment", "confidence"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::dataExtraction(
    const std::vector<std::string>& fields) {
    auto builder = OpenAIResponsesSchemaBuilder("data_extraction")
                       .description("Extract structured data from text");

    for (const auto& field : fields) {
        builder.property(field, JsonSchemaBuilder::string().description("Extracted " + field));
    }

    builder.required(fields);
    return builder.build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::classification(
    const std::vector<std::string>& categories) {
    return OpenAIResponsesSchemaBuilder("classification")
        .description("Classify the input into one of the predefined categories")
        .property("category", JsonSchemaBuilder::stringEnum(categories))
        .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
        .property("reasoning", JsonSchemaBuilder::string().description(
                                   "Brief explanation of the classification"))
        .required({"category", "confidence"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::summary(int maxLength) {
    return OpenAIResponsesSchemaBuilder("summary")
        .description("Generate a concise summary of the input text")
        .property("summary",
                  JsonSchemaBuilder::string().maxLength(maxLength).description("Concise summary"))
        .property(
            "key_points",
            JsonSchemaBuilder::arrayOf(JsonSchemaBuilder::string()).description("Main points"))
        .property("word_count",
                  JsonSchemaBuilder::integer().minimum(1).description("Number of words in summary"))
        .required({"summary", "key_points", "word_count"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::keyValueExtraction() {
    return OpenAIResponsesSchemaBuilder("key_value_extraction")
        .description("Extract key-value pairs from the text")
        .property("extracted_data",
                  JsonSchemaBuilder::object().additionalProperties(JsonSchemaBuilder::string()))
        .property("metadata", JsonSchemaBuilder::object()
                                  .property("extraction_confidence",
                                            JsonSchemaBuilder::number().minimum(0).maximum(1))
                                  .property("total_pairs", JsonSchemaBuilder::integer().minimum(0)))
        .required({"extracted_data", "metadata"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::booleanDecision(const std::string& question) {
    return OpenAIResponsesSchemaBuilder("boolean_decision")
        .description("Make a yes/no decision based on: " + question)
        .property("decision",
                  JsonSchemaBuilder::boolean().description("True for yes, false for no"))
        .property("reasoning",
                  JsonSchemaBuilder::string().description("Explanation for the decision"))
        .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
        .required({"decision", "reasoning", "confidence"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::entityExtraction() {
    auto entitySchema =
        JsonSchemaBuilder::object()
            .property("text", JsonSchemaBuilder::string().description("The extracted entity text"))
            .property("type", JsonSchemaBuilder::stringEnum(
                                  {"PERSON", "ORGANIZATION", "LOCATION", "DATE", "MONEY", "OTHER"}))
            .property("start_pos",
                      JsonSchemaBuilder::integer().minimum(0).description("Start position in text"))
            .property("end_pos",
                      JsonSchemaBuilder::integer().minimum(0).description("End position in text"))
            .required({"text", "type"});

    return OpenAIResponsesSchemaBuilder("entity_extraction")
        .description("Extract named entities from the text")
        .property("entities", JsonSchemaBuilder::arrayOf(entitySchema))
        .property("entity_count", JsonSchemaBuilder::integer().minimum(0))
        .required({"entities", "entity_count"})
        .build();
}

OpenAI::TextOutputConfig OpenAISchemaPatterns::translation(const std::string& targetLanguage) {
    return OpenAIResponsesSchemaBuilder("translation")
        .description("Translate text to " + targetLanguage)
        .property("translated_text", JsonSchemaBuilder::string().description("The translated text"))
        .property("source_language",
                  JsonSchemaBuilder::string().description("Detected source language"))
        .property("target_language", JsonSchemaBuilder::string().constValue(targetLanguage))
        .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
        .required({"translated_text", "source_language", "target_language", "confidence"})
        .build();
}

// Chat completions patterns
json OpenAISchemaPatterns::chatJsonMode() { return OpenAIChatSchemaBuilder().jsonMode().build(); }

json OpenAISchemaPatterns::chatClassification(const std::vector<std::string>& categories) {
    auto schema = JsonSchemaBuilder::object()
                      .property("category", JsonSchemaBuilder::stringEnum(categories))
                      .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
                      .required({"category", "confidence"});

    return OpenAIChatSchemaBuilder().jsonSchema("classification", schema).build();
}

json OpenAISchemaPatterns::chatSentiment() {
    auto schema = JsonSchemaBuilder::object()
                      .property("sentiment",
                                JsonSchemaBuilder::stringEnum({"positive", "negative", "neutral"}))
                      .property("confidence", JsonSchemaBuilder::number().minimum(0).maximum(1))
                      .required({"sentiment", "confidence"});

    return OpenAIChatSchemaBuilder().jsonSchema("sentiment", schema).build();
}
