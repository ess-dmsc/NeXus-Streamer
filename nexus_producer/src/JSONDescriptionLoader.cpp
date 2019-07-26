#include <fstream>
#include <nlohmann/json.hpp>
#include <streambuf>

#include "JSONDescriptionLoader.h"

namespace JSONDescriptionLoader {
void checkIsValidJson(const std::string &textForValidation) {
  auto jsonObject = nlohmann::json::parse(textForValidation);
}

std::string loadFromFile(const std::string &filepath) {
  std::ifstream filestream(filepath);
  std::string json_string;

  filestream.seekg(0, std::ios::end);
  json_string.reserve(filestream.tellg());
  filestream.seekg(0, std::ios::beg);

  json_string.assign((std::istreambuf_iterator<char>(filestream)),
                     std::istreambuf_iterator<char>());

  checkIsValidJson(json_string);
  return json_string;
}
}
