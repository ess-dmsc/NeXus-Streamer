#include <nlohmann/json.hpp>

#include "JSONDescriptionLoader.h"

namespace JSONDescriptionLoader {
void checkIsValidJson(const std::string &textForValidation) {
  auto jsonObject = nlohmann::json::parse(textForValidation);
}

std::string loadFromFile(const std::string &filepath) {
  const std::string json_string = "{}";
  checkIsValidJson(json_string);
  return json_string;
}
}
