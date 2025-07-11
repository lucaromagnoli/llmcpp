#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;

/**
 * @brief Fluent API for building JSON schemas programmatically
 *
 * Provides a chainable interface for creating JSON schemas for validation
 * and structured output. Supports all common JSON Schema Draft 7 features.
 *
 * Example usage:
 * @code
 * auto schema = JsonSchemaBuilder()
 *     .type("object")
 *     .property("name", JsonSchemaBuilder().type("string").required())
 *     .property("age", JsonSchemaBuilder().type("integer").minimum(0))
 *     .required({"name", "age"})
 *     .build();
 * @endcode
 */
class JsonSchemaBuilder {
   public:
    JsonSchemaBuilder() = default;

    // Core schema properties
    JsonSchemaBuilder& type(const std::string& type);
    JsonSchemaBuilder& title(const std::string& title);
    JsonSchemaBuilder& description(const std::string& description);
    JsonSchemaBuilder& defaultValue(const json& value);

    // String constraints
    JsonSchemaBuilder& minLength(int min);
    JsonSchemaBuilder& maxLength(int max);
    JsonSchemaBuilder& pattern(const std::string& regex);
    JsonSchemaBuilder& format(const std::string& format);  // e.g., "email", "date-time"

    // Numeric constraints
    JsonSchemaBuilder& minimum(double min);
    JsonSchemaBuilder& maximum(double max);
    JsonSchemaBuilder& exclusiveMinimum(double min);
    JsonSchemaBuilder& exclusiveMaximum(double max);
    JsonSchemaBuilder& multipleOf(double value);

    // Array constraints
    JsonSchemaBuilder& minItems(int min);
    JsonSchemaBuilder& maxItems(int max);
    JsonSchemaBuilder& uniqueItems(bool unique = true);
    JsonSchemaBuilder& items(const JsonSchemaBuilder& itemSchema);

    // Object constraints
    JsonSchemaBuilder& property(const std::string& name, const JsonSchemaBuilder& propSchema);
    JsonSchemaBuilder& required(const std::vector<std::string>& requiredProps);
    JsonSchemaBuilder& additionalProperties(bool allowed);
    JsonSchemaBuilder& additionalProperties(const JsonSchemaBuilder& schema);
    JsonSchemaBuilder& minProperties(int min);
    JsonSchemaBuilder& maxProperties(int max);

    // Enumeration
    JsonSchemaBuilder& enumValues(const std::vector<json>& values);
    template <typename T>
    JsonSchemaBuilder& enumValues(const std::vector<T>& values);

    // Composition
    JsonSchemaBuilder& anyOf(const std::vector<JsonSchemaBuilder>& schemas);
    JsonSchemaBuilder& oneOf(const std::vector<JsonSchemaBuilder>& schemas);
    JsonSchemaBuilder& allOf(const std::vector<JsonSchemaBuilder>& schemas);
    JsonSchemaBuilder& notSchema(const JsonSchemaBuilder& schema);

    // Conditional schemas
    JsonSchemaBuilder& ifThen(const JsonSchemaBuilder& ifSchema,
                              const JsonSchemaBuilder& thenSchema);
    JsonSchemaBuilder& ifThenElse(const JsonSchemaBuilder& ifSchema,
                                  const JsonSchemaBuilder& thenSchema,
                                  const JsonSchemaBuilder& elseSchema);

    // Examples and constants
    JsonSchemaBuilder& examples(const std::vector<json>& examples);
    JsonSchemaBuilder& constValue(const json& value);

    // Build the final schema
    json build() const;

    // Utility methods
    static JsonSchemaBuilder object();
    static JsonSchemaBuilder array();
    static JsonSchemaBuilder string();
    static JsonSchemaBuilder integer();
    static JsonSchemaBuilder number();
    static JsonSchemaBuilder boolean();
    static JsonSchemaBuilder null();

    // Common patterns
    static JsonSchemaBuilder stringEnum(const std::vector<std::string>& values);
    static JsonSchemaBuilder arrayOf(const JsonSchemaBuilder& itemSchema);
    static JsonSchemaBuilder optionalString();
    static JsonSchemaBuilder requiredString();

   private:
    json schema_;

    // Helper methods
    void setIfNotEmpty(const std::string& key, const json& value);
    std::vector<json> buildersToJson(const std::vector<JsonSchemaBuilder>& builders) const;
};

// Template implementation
template <typename T>
JsonSchemaBuilder& JsonSchemaBuilder::enumValues(const std::vector<T>& values) {
    std::vector<json> jsonValues;
    jsonValues.reserve(values.size());
    for (const auto& value : values) {
        jsonValues.push_back(json(value));
    }
    return enumValues(jsonValues);
}
