#pragma once

#include <string>

namespace JSONDescriptionLoader {

void checkIsValidJson(const std::string &textForValidation);
std::string loadJsonFromFile(const std::string &filepath);
void replaceString(std::string &input, const std::string &search,
                   const std::string &replacement);
void updateTopicNames(std::string &description,
                      const std::string &instrumentName);
} // namespace JSONDescriptionLoader
