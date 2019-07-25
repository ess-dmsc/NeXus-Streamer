#include <gtest/gtest.h>

#include "JSONDescriptionLoader.h"

class JSONDescriptionLoaderTest : public ::testing::Test {};

TEST(JSONDescriptionLoaderTest, test_JsonDescriptionIsValid_reports_false_for_empty_string) {
  std::string emptyString;
  ASSERT_EQ(JsonDescriptionIsValid(emptyString), false);
}
