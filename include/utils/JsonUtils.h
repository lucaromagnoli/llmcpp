#pragma once
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

/**
 * Utility function for safely extracting JSON values with default fallbacks
 * Similar to Python's dict.get() method
 */
template <typename T>
T safeGetJson(const json& j, const std::string& key, const T& defaultValue = T{}) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return defaultValue;
}

/**
 * Utility function for safely extracting optional JSON values
 * Returns std::nullopt if key doesn't exist or is null
 */
template <typename T>
std::optional<T> safeGetOptionalJson(const json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<T>();
    }
    return std::nullopt;
}

/**
 * Utility function for safely extracting required JSON values
 * Throws std::runtime_error with helpful message if key is missing or null
 */
template <typename T>
T safeGetRequiredJson(const json& j, const std::string& key) {
    if (!j.contains(key)) {
        throw std::runtime_error("Required JSON key '" + key + "' is missing");
    }
    if (j[key].is_null()) {
        throw std::runtime_error("Required JSON key '" + key + "' is null");
    }
    return j[key].get<T>();
}
