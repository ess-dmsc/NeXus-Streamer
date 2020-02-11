#include <6s4t_run_stop_generated.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <pl72_run_start_generated.h>

#include "RunData.h"

class RunDataTest : public ::testing::Test {};

TEST(RunDataTest, set_and_get_start_time) {
  auto runData = RunData();
  runData.setStartTimeFromString("2016-08-11T08:50:18");
  EXPECT_EQ(1470905418000000000, runData.startTime);

  runData.setStartTimeInSeconds(1470905418);
  EXPECT_EQ(1470905418000000000, runData.startTime);
}

TEST(RunDataTest, set_and_get_stop_time) {
  auto runData = RunData();
  runData.setStopTimeFromString("2016-08-13T13:32:09");
  EXPECT_EQ(1471095129000000000, runData.stopTime);
}

TEST(RunDataTest, get_run_info_as_string) {
  auto runData = RunData();
  runData.instrumentName = "SANS2D";
  runData.runID = "42";
  runData.setStartTimeFromString("2016-08-11T08:50:18");

  EXPECT_EQ("Run ID: 42, Instrument name: SANS2D, Start time: "
            "2016-08-11T08:50:18",
            fmt::format("{}", runData));
}

TEST(RunDataTest, encode_and_decode_run_start) {
  auto inputRunData = RunData();
  inputRunData.setStartTimeFromString("2016-08-11T08:50:18");
  inputRunData.setStopTimeFromString("2016-08-11T08:50:18");
  inputRunData.runID = "42";
  inputRunData.instrumentName = "SANS2D";
  inputRunData.nexusStructure = "{}";
  inputRunData.jobID = "job42";
  inputRunData.serviceID = "service42";
  inputRunData.numberOfPeriods = 3;


  auto runStartMessage = serialiseRunStartMessage(inputRunData);
  auto outputRunData = deserialiseRunStartMessage(
      reinterpret_cast<const uint8_t *>(runStartMessage.data()));

  EXPECT_EQ(outputRunData.startTime, inputRunData.startTime);
  EXPECT_EQ(outputRunData.stopTime, inputRunData.stopTime);
  EXPECT_EQ(outputRunData.runID, inputRunData.runID);
  EXPECT_EQ(outputRunData.instrumentName, inputRunData.instrumentName);
  EXPECT_EQ(outputRunData.nexusStructure, inputRunData.nexusStructure);
  EXPECT_EQ(outputRunData.jobID, inputRunData.jobID);
  EXPECT_EQ(outputRunData.serviceID, inputRunData.serviceID);
  EXPECT_EQ(outputRunData.numberOfPeriods, inputRunData.numberOfPeriods);
}

TEST(RunDataTest, encode_and_decode_run_stop) {
  auto inputRunData = RunData();
  inputRunData.setStopTimeFromString("2016-08-11T08:50:18");
  inputRunData.runID = "42";
  inputRunData.jobID = "job42";
  inputRunData.serviceID = "service42";

  auto runStopMessage = serialiseRunStopMessage(inputRunData);
  auto outputRunData = deserialiseRunStartMessage(
      reinterpret_cast<const uint8_t *>(runStopMessage.data()));

  EXPECT_EQ(outputRunData.stopTime, inputRunData.stopTime);
  EXPECT_EQ(outputRunData.runID, inputRunData.runID);
  EXPECT_EQ(outputRunData.jobID, inputRunData.jobID);
  EXPECT_EQ(outputRunData.serviceID, inputRunData.serviceID);
}

TEST(RunDataTest, check_stop_message_includes_file_identifier) {
  auto runData = RunData();
  auto runMessage = serialiseRunStopMessage(runData);
  auto runIdentifier = RunStopIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(runMessage.data()), runIdentifier));
}

TEST(RunDataTest, check_start_message_includes_file_identifier) {
  auto runData = RunData();
  auto runMessage = serialiseRunStartMessage(runData);
  auto runIdentifier = RunStartIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(runMessage.data()), runIdentifier));
}
