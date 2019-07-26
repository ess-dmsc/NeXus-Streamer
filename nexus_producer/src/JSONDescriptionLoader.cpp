#include <fstream>
#include <nlohmann/json.hpp>
#include <streambuf>

#include "JSONDescriptionLoader.h"

namespace JSONDescriptionLoader {
void checkIsValidJson(const std::string &textForValidation) {
  auto jsonObject = nlohmann::json::parse(textForValidation);
}

void replaceString(std::string &input, const std::string &search,
                   const std::string &replace) {
  size_t position = 0;
  while ((position = input.find(search, position)) != std::string::npos) {
    input.replace(position, search.length(), replace);
    position += replace.length();
  }
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

std::string loadJsonDescription(const std::string &filepath,
                                const std::string &sampleEnvTopic,
                                const std::string &eventTopic,
                                const std::string &histoTopic) {
  std::string description = loadFromFile(filepath);
  replaceString(description, "SAMPLE_ENV_TOPIC", sampleEnvTopic);
  replaceString(description, "EVENT_DATA_TOPIC", eventTopic);
  replaceString(description, "HISTO_DATA_TOPIC", histoTopic);
  return description;
}
}
