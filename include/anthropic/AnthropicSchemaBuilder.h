#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace Anthropic {

/**
 * @brief Schema builder for Anthropic tool definitions
 *
 * Builds JSON schemas compatible with Anthropic's tool use API
 * following JSON Schema draft 2020-12 specification
 */
class SchemaBuilder {
   private:
    json schema_;

   public:
    SchemaBuilder();

    // Chainable builder methods
    SchemaBuilder& setType(const std::string& type);
    SchemaBuilder& setDescription(const std::string& description);
    SchemaBuilder& setMinimum(double minimum);
    SchemaBuilder& setMaximum(double maximum);
    SchemaBuilder& setMinLength(int minLength);
    SchemaBuilder& setMaxLength(int maxLength);
    SchemaBuilder& setMinItems(int minItems);
    SchemaBuilder& setMaxItems(int maxItems);
    SchemaBuilder& setRequired(const std::vector<std::string>& required);
    SchemaBuilder& setAdditionalProperties(bool allowed);
    SchemaBuilder& setProperties(const json& properties);
    SchemaBuilder& setItems(const json& itemSchema);

    // Build final schema
    json build() const;

    // Static convenience methods for common schemas
    static json buildMusicalSequenceSchema();
    static json buildObjectSchema(const json& properties,
                                  const std::vector<std::string>& required = {});
    static json buildArraySchema(const json& itemSchema, int minItems = -1, int maxItems = -1);
    static json buildStringSchema(const std::string& description = "", int minLength = -1,
                                  int maxLength = -1);
    static json buildIntegerSchema(const std::string& description = "", int minimum = INT_MIN,
                                   int maximum = INT_MAX);
    static json buildNumberSchema(const std::string& description = "", double minimum = -INFINITY,
                                  double maximum = INFINITY);
};

}  // namespace Anthropic
