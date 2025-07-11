#include "core/JsonSchemaBuilder.h"

// Core schema properties
JsonSchemaBuilder& JsonSchemaBuilder::type(const std::string& type) {
    schema_["type"] = type;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::title(const std::string& title) {
    schema_["title"] = title;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::description(const std::string& description) {
    schema_["description"] = description;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::defaultValue(const json& value) {
    schema_["default"] = value;
    return *this;
}

// String constraints
JsonSchemaBuilder& JsonSchemaBuilder::minLength(int min) {
    schema_["minLength"] = min;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::maxLength(int max) {
    schema_["maxLength"] = max;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::pattern(const std::string& regex) {
    schema_["pattern"] = regex;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::format(const std::string& format) {
    schema_["format"] = format;
    return *this;
}

// Numeric constraints
JsonSchemaBuilder& JsonSchemaBuilder::minimum(double min) {
    schema_["minimum"] = min;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::maximum(double max) {
    schema_["maximum"] = max;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::exclusiveMinimum(double min) {
    schema_["exclusiveMinimum"] = min;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::exclusiveMaximum(double max) {
    schema_["exclusiveMaximum"] = max;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::multipleOf(double value) {
    schema_["multipleOf"] = value;
    return *this;
}

// Array constraints
JsonSchemaBuilder& JsonSchemaBuilder::minItems(int min) {
    schema_["minItems"] = min;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::maxItems(int max) {
    schema_["maxItems"] = max;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::uniqueItems(bool unique) {
    schema_["uniqueItems"] = unique;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::items(const JsonSchemaBuilder& itemSchema) {
    schema_["items"] = itemSchema.build();
    return *this;
}

// Object constraints
JsonSchemaBuilder& JsonSchemaBuilder::property(const std::string& name,
                                               const JsonSchemaBuilder& propSchema) {
    if (!schema_.contains("properties")) {
        schema_["properties"] = json::object();
    }
    schema_["properties"][name] = propSchema.build();
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::required(const std::vector<std::string>& requiredProps) {
    schema_["required"] = requiredProps;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::additionalProperties(bool allowed) {
    schema_["additionalProperties"] = allowed;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::additionalProperties(const JsonSchemaBuilder& schema) {
    schema_["additionalProperties"] = schema.build();
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::minProperties(int min) {
    schema_["minProperties"] = min;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::maxProperties(int max) {
    schema_["maxProperties"] = max;
    return *this;
}

// Enumeration
JsonSchemaBuilder& JsonSchemaBuilder::enumValues(const std::vector<json>& values) {
    schema_["enum"] = values;
    return *this;
}

// Composition
JsonSchemaBuilder& JsonSchemaBuilder::anyOf(const std::vector<JsonSchemaBuilder>& schemas) {
    schema_["anyOf"] = buildersToJson(schemas);
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::oneOf(const std::vector<JsonSchemaBuilder>& schemas) {
    schema_["oneOf"] = buildersToJson(schemas);
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::allOf(const std::vector<JsonSchemaBuilder>& schemas) {
    schema_["allOf"] = buildersToJson(schemas);
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::notSchema(const JsonSchemaBuilder& schema) {
    schema_["not"] = schema.build();
    return *this;
}

// Conditional schemas
JsonSchemaBuilder& JsonSchemaBuilder::ifThen(const JsonSchemaBuilder& ifSchema,
                                             const JsonSchemaBuilder& thenSchema) {
    schema_["if"] = ifSchema.build();
    schema_["then"] = thenSchema.build();
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::ifThenElse(const JsonSchemaBuilder& ifSchema,
                                                 const JsonSchemaBuilder& thenSchema,
                                                 const JsonSchemaBuilder& elseSchema) {
    schema_["if"] = ifSchema.build();
    schema_["then"] = thenSchema.build();
    schema_["else"] = elseSchema.build();
    return *this;
}

// Examples and constants
JsonSchemaBuilder& JsonSchemaBuilder::examples(const std::vector<json>& examples) {
    schema_["examples"] = examples;
    return *this;
}

JsonSchemaBuilder& JsonSchemaBuilder::constValue(const json& value) {
    schema_["const"] = value;
    return *this;
}

// Build the final schema
json JsonSchemaBuilder::build() const { return schema_; }

// Utility methods
JsonSchemaBuilder JsonSchemaBuilder::object() { return JsonSchemaBuilder().type("object"); }

JsonSchemaBuilder JsonSchemaBuilder::array() { return JsonSchemaBuilder().type("array"); }

JsonSchemaBuilder JsonSchemaBuilder::string() { return JsonSchemaBuilder().type("string"); }

JsonSchemaBuilder JsonSchemaBuilder::integer() { return JsonSchemaBuilder().type("integer"); }

JsonSchemaBuilder JsonSchemaBuilder::number() { return JsonSchemaBuilder().type("number"); }

JsonSchemaBuilder JsonSchemaBuilder::boolean() { return JsonSchemaBuilder().type("boolean"); }

JsonSchemaBuilder JsonSchemaBuilder::null() { return JsonSchemaBuilder().type("null"); }

// Common patterns
JsonSchemaBuilder JsonSchemaBuilder::stringEnum(const std::vector<std::string>& values) {
    std::vector<json> jsonValues;
    jsonValues.reserve(values.size());
    for (const auto& value : values) {
        jsonValues.push_back(json(value));
    }
    return JsonSchemaBuilder().type("string").enumValues(jsonValues);
}

JsonSchemaBuilder JsonSchemaBuilder::arrayOf(const JsonSchemaBuilder& itemSchema) {
    return JsonSchemaBuilder().type("array").items(itemSchema);
}

JsonSchemaBuilder JsonSchemaBuilder::optionalString() { return JsonSchemaBuilder().type("string"); }

JsonSchemaBuilder JsonSchemaBuilder::requiredString() { return JsonSchemaBuilder().type("string"); }

// Helper methods
void JsonSchemaBuilder::setIfNotEmpty(const std::string& key, const json& value) {
    if (!value.is_null()) {
        schema_[key] = value;
    }
}

std::vector<json> JsonSchemaBuilder::buildersToJson(
    const std::vector<JsonSchemaBuilder>& builders) const {
    std::vector<json> result;
    result.reserve(builders.size());
    for (const auto& builder : builders) {
        result.push_back(builder.build());
    }
    return result;
}
