#include <nlohmann/json.hpp>

#include "JSONDescriptionLoader.h"

void checkIsValidJson(const std::string &textForValidation) {
  auto jsonObject = nlohmann::json::parse(textForValidation);
}
