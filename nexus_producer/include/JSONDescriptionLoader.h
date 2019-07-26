#pragma once

#include <string>

namespace JSONDescriptionLoader {

void checkIsValidJson(const std::string &textForValidation);
std::string loadFromFile(const std::string &filepath);

}