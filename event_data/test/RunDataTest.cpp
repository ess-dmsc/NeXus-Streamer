#include "RunData.h"
#include <gtest/gtest.h>

class RunDataTest : public ::testing::Test {};

TEST(RunDataTest, set_and_get_start_time) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStartTime("2016-08-11T08:50:18"));
  EXPECT_EQ(1470905418000000000, rundata.getStartTime());

  EXPECT_NO_THROW(rundata.setStartTimeInSeconds(1470905418));
  EXPECT_EQ(1470905418000000000, rundata.getStartTime());
}

TEST(RunDataTest, set_and_get_stop_time) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStopTime("2016-08-13T13:32:09"));
  EXPECT_EQ(1471095129000000000, rundata.getStopTime());

  EXPECT_NO_THROW(rundata.setStopTime(1471095129000000000));
  EXPECT_EQ(1471095129000000000, rundata.getStopTime());
}

TEST(RunDataTest, set_and_get_run_number) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setRunNumber(42));
  EXPECT_EQ(42, rundata.getRunNumber());
}

TEST(RunDataTest, set_and_get_instrument_name) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setInstrumentName("SANS2D"));
  EXPECT_EQ("SANS2D", rundata.getInstrumentName());
}

TEST(RunDataTest, get_RunInfo) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setInstrumentName("SANS2D"));
  EXPECT_NO_THROW(rundata.setRunNumber(42));
  EXPECT_NO_THROW(rundata.setStartTime("2016-08-11T08:50:18"));

  EXPECT_EQ("Run number: 42, Instrument name: SANS2D, Start time: "
            "2016-08-11T08:50:18",
            rundata.runInfo());
}

TEST(RunDataTest, encode_and_decode_RunData) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setInstrumentName("SANS2D"));
  EXPECT_NO_THROW(rundata.setRunNumber(42));
  EXPECT_NO_THROW(rundata.setStartTime("2016-08-11T08:50:18"));
  EXPECT_NO_THROW(rundata.setNumberOfPeriods(1));

  auto buffer = rundata.getRunStartBuffer();

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(
      reinterpret_cast<const uint8_t *>(buffer.data())));
  EXPECT_EQ(42, receivedRunData.getRunNumber());
  EXPECT_EQ("SANS2D", receivedRunData.getInstrumentName());
  EXPECT_EQ(1470905418000000000, receivedRunData.getStartTime());
  EXPECT_EQ(1, receivedRunData.getNumberOfPeriods());
}

TEST(RunDataTest, encode_and_decode_RunStop) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStopTime("2016-08-11T08:50:18"));

  auto buffer = rundata.getRunStopBuffer();

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(
      reinterpret_cast<const uint8_t *>(buffer.data())));
  EXPECT_EQ(1470905418000000000, receivedRunData.getStopTime());
}

TEST(RunDataTest, check_buffer_includes_file_identifier) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStopTime("2016-08-11T08:50:18"));

  auto buffer = rundata.getRunStopBuffer();

  auto runIdentifier = RunInfoIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(buffer.data()), runIdentifier));
}
