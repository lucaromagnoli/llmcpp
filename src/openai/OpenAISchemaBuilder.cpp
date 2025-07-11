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
