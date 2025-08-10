#include "anthropic/AnthropicSchemaBuilder.h"

#include <climits>
#include <cmath>

namespace Anthropic {

SchemaBuilder::SchemaBuilder() { schema_ = json::object(); }

SchemaBuilder& SchemaBuilder::setType(const std::string& type) {
    schema_["type"] = type;
    return *this;
}

SchemaBuilder& SchemaBuilder::setDescription(const std::string& description) {
    if (!description.empty()) {
        schema_["description"] = description;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMinimum(double minimum) {
    if (minimum != -INFINITY) {
        schema_["minimum"] = minimum;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMaximum(double maximum) {
    if (maximum != INFINITY) {
        schema_["maximum"] = maximum;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMinLength(int minLength) {
    if (minLength >= 0) {
        schema_["minLength"] = minLength;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMaxLength(int maxLength) {
    if (maxLength >= 0) {
        schema_["maxLength"] = maxLength;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMinItems(int minItems) {
    if (minItems >= 0) {
        schema_["minItems"] = minItems;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setMaxItems(int maxItems) {
    if (maxItems >= 0) {
        schema_["maxItems"] = maxItems;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setRequired(const std::vector<std::string>& required) {
    if (!required.empty()) {
        schema_["required"] = required;
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::setAdditionalProperties(bool allowed) {
    schema_["additionalProperties"] = allowed;
    return *this;
}

SchemaBuilder& SchemaBuilder::setProperties(const json& properties) {
    schema_["properties"] = properties;
    return *this;
}

SchemaBuilder& SchemaBuilder::setItems(const json& itemSchema) {
    schema_["items"] = itemSchema;
    return *this;
}

json SchemaBuilder::build() const { return schema_; }

// Static convenience methods

json SchemaBuilder::buildMusicalSequenceSchema() {
    // Build note schema
    json noteSchema =
        SchemaBuilder()
            .setType("object")
            .setAdditionalProperties(false)
            .setProperties({{"note", buildIntegerSchema("MIDI note number", 21, 127)},
                            {"start", buildNumberSchema("Start time in beats", 0)},
                            {"duration", buildNumberSchema("Note duration in beats", 0.1)},
                            {"velocity", buildIntegerSchema("Note velocity (volume)", 1, 127)}})
            .setRequired({"note", "start", "duration", "velocity"})
            .build();

    // Build sequence data array schema
    json sequenceDataSchema = SchemaBuilder()
                                  .setType("array")
                                  .setDescription("Array of musical notes")
                                  .setItems(noteSchema)
                                  .build();

    // Build main schema
    return SchemaBuilder()
        .setType("object")
        .setAdditionalProperties(false)
        .setProperties({{"sequence_data", sequenceDataSchema},
                        {"description", buildStringSchema("Description of the musical sequence")}})
        .setRequired({"sequence_data", "description"})
        .build();
}

json SchemaBuilder::buildObjectSchema(const json& properties,
                                      const std::vector<std::string>& required) {
    return SchemaBuilder()
        .setType("object")
        .setProperties(properties)
        .setRequired(required)
        .setAdditionalProperties(false)
        .build();
}

json SchemaBuilder::buildArraySchema(const json& itemSchema, int minItems, int maxItems) {
    return SchemaBuilder()
        .setType("array")
        .setItems(itemSchema)
        .setMinItems(minItems)
        .setMaxItems(maxItems)
        .build();
}

json SchemaBuilder::buildStringSchema(const std::string& description, int minLength,
                                      int maxLength) {
    return SchemaBuilder()
        .setType("string")
        .setDescription(description)
        .setMinLength(minLength)
        .setMaxLength(maxLength)
        .build();
}

json SchemaBuilder::buildIntegerSchema(const std::string& description, int minimum, int maximum) {
    return SchemaBuilder()
        .setType("integer")
        .setDescription(description)
        .setMinimum(minimum != INT_MIN ? minimum : -INFINITY)
        .setMaximum(maximum != INT_MAX ? maximum : INFINITY)
        .build();
}

json SchemaBuilder::buildNumberSchema(const std::string& description, double minimum,
                                      double maximum) {
    return SchemaBuilder()
        .setType("number")
        .setDescription(description)
        .setMinimum(minimum)
        .setMaximum(maximum)
        .build();
}

}  // namespace Anthropic
