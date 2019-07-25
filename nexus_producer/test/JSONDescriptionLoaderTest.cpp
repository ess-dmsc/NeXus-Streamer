#include <gtest/gtest.h>

#include "JSONDescriptionLoader.h"

class JSONDescriptionLoaderTest : public ::testing::Test {};

TEST(JSONDescriptionLoaderTest, test_JsonDescriptionIsValid_reports_false_for_empty_string) {
  std::string emptyString;
  ASSERT_EQ(JsonDescriptionIsValid(emptyString), false);
}

TEST(JSONDescriptionLoaderTest, test_JsonDescriptionIsValid_reports_false_for_invalid_json) {
  std::string invalidJson = R"(["field" : 123])";
  ASSERT_EQ(JsonDescriptionIsValid(invalidJson), false);
}

TEST(JSONDescriptionLoaderTest, test_JsonDescriptionIsValid_reports_true_for_valid_json) {
  std::string validJson = R"({"field" : 123})";
  ASSERT_EQ(JsonDescriptionIsValid(validJson), true);
}
