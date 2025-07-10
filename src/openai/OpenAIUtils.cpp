#include "openai/OpenAIUtils.h"
#include <stdexcept>

namespace OpenAI {

std::string buildUrl(const std::string& endpoint) {
    return "https://api.openai.com/v1" + endpoint;
}

json createHeaders(const std::string& apiKey) {
    return json{
        {"Authorization", "Bearer " + apiKey},
        {"Content-Type", "application/json"}
    };
}

} // namespace OpenAI 