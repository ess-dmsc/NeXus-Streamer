#pragma once

#include <string>

namespace JSONDescriptionLoader {

void checkIsValidJson(const std::string &textForValidation);
std::string loadFromFile(const std::string &filepath);
void replaceString(std::string &input, const std::string &search,
                   const std::string &replace);
std::string loadJsonDescription(const std::string &filepath,
                                const std::string &instrumentName);
}