#include <gtest/gtest.h>

#include "JSONDescriptionLoader.h"

class JSONDescriptionLoaderTest : public ::testing::Test {};

TEST(JSONDescriptionLoaderTest, isValidJson_reports_false_for_empty_string) {
  std::string emptyString;
  ASSERT_EQ(isValidJson(emptyString), false);
}

TEST(JSONDescriptionLoaderTest, isValidJson_reports_false_for_invalid_json) {
  std::string invalidJson = R"(["field" : 123])";  // invalid as no braces
  ASSERT_EQ(isValidJson(invalidJson), false);
}

TEST(JSONDescriptionLoaderTest, isValidJson_reports_true_for_valid_json) {
  std::string validJson = R"({"field" : 123})";
  ASSERT_EQ(isValidJson(validJson), true);
}
