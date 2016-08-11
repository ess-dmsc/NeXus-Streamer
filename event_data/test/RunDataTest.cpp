#include "RunData.h"
#include <gtest/gtest.h>

class RunDataTest : public ::testing::Test {};

TEST(RunDataTest, set_and_get_start_time) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStartTime("2016-08-11T08:50:18"));
  EXPECT_EQ(1470905418, rundata.getStartTime());

  EXPECT_NO_THROW(rundata.setStartTime(1470905418));
  EXPECT_EQ(1470905418, rundata.getStartTime());
}

TEST(RunDataTest, set_and_get_run_number) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setRunNumber(42));
  EXPECT_EQ(42, rundata.getRunNumber());
}

TEST(RunDataTest, set_and_get_stream_offset) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStreamOffset(42));
  EXPECT_EQ(42, rundata.getStreamOffset());
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
            "2016-08-11T09:50:18",
            rundata.runInfo());
}

TEST(RunDataTest, encode_and_decode_RunData) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setInstrumentName("SANS2D"));
  EXPECT_NO_THROW(rundata.setRunNumber(42));
  EXPECT_NO_THROW(rundata.setStartTime("2016-08-11T08:50:18"));

  std::string rawbuf;
  EXPECT_NO_THROW(rundata.getBufferPointer(rawbuf));

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(42, receivedRunData.getRunNumber());
  EXPECT_EQ("SANS2D", receivedRunData.getInstrumentName());
  EXPECT_EQ(1470905418, receivedRunData.getStartTime());
}
