#include <nlohmann/json.hpp>

#include "JSONDescriptionLoader.h"

bool JsonDescriptionIsValid(const std::string &textForValidation) {
  try {
    auto jsonLoaded = nlohmann::json::parse(textForValidation);
  } catch (const nlohmann::json::parse_error &e) {
    return false;
  }

  return true;
}
