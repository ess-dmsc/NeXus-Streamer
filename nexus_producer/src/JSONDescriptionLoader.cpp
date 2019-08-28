#include <fstream>
#include <nlohmann/json.hpp>
#include <streambuf>

#include "JSONDescriptionLoader.h"
#include "TopicNames.h"

namespace JSONDescriptionLoader {
void checkIsValidJson(const std::string &textForValidation) {
  auto jsonObject = nlohmann::json::parse(textForValidation);
}

/**
 * Replace all occurrences of a substring in a string
 * @param input string
 * @param search substring to search for
 * @param replacement string
 */
void replaceString(std::string &input, const std::string &search,
                   const std::string &replacement) {
  size_t position = 0;
  while ((position = input.find(search, position)) != std::string::npos) {
    input.replace(position, search.length(), replacement);
    position += replacement.length();
  }
}

std::string loadJsonFromFile(const std::string &filepath) {
  std::ifstream filestream(filepath);
  std::string json_string;

  if (filepath.empty()) {
    return json_string;
  }

  filestream.seekg(0, std::ios::end);
  json_string.reserve(static_cast<size_t>(filestream.tellg()));
  filestream.seekg(0, std::ios::beg);

  json_string.assign((std::istreambuf_iterator<char>(filestream)),
                     std::istreambuf_iterator<char>());

  checkIsValidJson(json_string);
  return json_string;
}

/**
 * Replace placeholder topic names inserted by the python script
 * @param description The read contents of a file containing the JSON
 * description of the NeXus file, modified by function
 * @param instrumentName The instrument name, used as a prefix in the topic name
 * @return JSON description with correct topic names in any "stream" objects
 */
void updateTopicNames(std::string &description,
                      const std::string &instrumentName) {
  auto topicNames = TopicNames(instrumentName);
  replaceString(description, "SAMPLE_ENV_TOPIC", topicNames.sampleEnv);
  replaceString(description, "EVENT_DATA_TOPIC", topicNames.event);
  replaceString(description, "HISTO_DATA_TOPIC", topicNames.histogram);
}
}
