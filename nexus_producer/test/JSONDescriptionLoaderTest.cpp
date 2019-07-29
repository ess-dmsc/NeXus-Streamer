#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "JSONDescriptionLoader.h"

class JSONDescriptionLoaderTest : public ::testing::Test {};

TEST(JSONDescriptionLoaderTest, isValidJson_reports_false_for_empty_string) {
  std::string emptyString;
  ASSERT_THROW(JSONDescriptionLoader::checkIsValidJson(emptyString),
               nlohmann::json::parse_error);
}

TEST(JSONDescriptionLoaderTest, isValidJson_reports_false_for_invalid_json) {
  std::string invalidJson = R"(["field" : 123])"; // invalid as no braces
  ASSERT_THROW(JSONDescriptionLoader::checkIsValidJson(invalidJson),
               nlohmann::json::parse_error);
}

TEST(JSONDescriptionLoaderTest, isValidJson_reports_true_for_valid_json) {
  std::string validJson = R"({"field" : 123})";
  ASSERT_NO_THROW(JSONDescriptionLoader::checkIsValidJson(validJson));
}

TEST(JSONDescriptionLoaderTest,
     replaceString_replaces_single_text_match_in_string) {
  std::string inputString = "Hello all";
  JSONDescriptionLoader::replaceString(inputString, "all", "world!");
  ASSERT_EQ(inputString, "Hello world!");
}

TEST(JSONDescriptionLoaderTest,
     replaceString_replaces_multiple_text_matches_in_string) {
  std::string inputString = "??? con???enate ???erpillar";
  JSONDescriptionLoader::replaceString(inputString, "???", "cat");
  ASSERT_EQ(inputString, "cat concatenate caterpillar");
}

TEST(JSONDescriptionLoaderTest,
     test_updateTopicNames_replaces_topic_placeholders) {

  const std::string instrumentName = "INST";

  const std::string sampleEnvTopic = "SAMPLE_ENV_TOPIC";
  std::string sampleEnvTopicModify = sampleEnvTopic;
  JSONDescriptionLoader::updateTopicNames(sampleEnvTopicModify, instrumentName);
  ASSERT_TRUE(sampleEnvTopicModify.find(sampleEnvTopic) == std::string::npos)
      << "Expected text to no longer contain topic placeholder";
  ASSERT_TRUE(sampleEnvTopicModify.find("INST") != std::string::npos)
      << "Expected text to now contain instrument name";

  const std::string eventDataTopic = "EVENT_DATA_TOPIC";
  std::string eventDataTopicModify = eventDataTopic;
  JSONDescriptionLoader::updateTopicNames(eventDataTopicModify, instrumentName);
  ASSERT_TRUE(eventDataTopicModify.find(eventDataTopic) == std::string::npos)
      << "Expected text to no longer contain topic placeholder";
  ASSERT_TRUE(eventDataTopicModify.find("INST") != std::string::npos)
      << "Expected text to now contain instrument name";

  const std::string histDataTopic = "HISTO_DATA_TOPIC";
  std::string histDataTopicModify = histDataTopic;
  JSONDescriptionLoader::updateTopicNames(histDataTopicModify, instrumentName);
  ASSERT_TRUE(histDataTopicModify.find(histDataTopic) == std::string::npos)
      << "Expected text to no longer contain topic placeholder";
  ASSERT_TRUE(histDataTopicModify.find("INST") != std::string::npos)
      << "Expected text to now contain instrument name";
}
