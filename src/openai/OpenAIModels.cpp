#include "openai/OpenAIModels.h"

#include <string>
#include <vector>

namespace OpenAI {

std::vector<std::string> getAllModels() {
    std::vector<std::string> models;
    models.insert(models.end(), RESPONSES_MODELS.begin(), RESPONSES_MODELS.end());
    models.insert(models.end(), CHAT_COMPLETION_MODELS.begin(), CHAT_COMPLETION_MODELS.end());
    return models;
}

}  // namespace OpenAI
