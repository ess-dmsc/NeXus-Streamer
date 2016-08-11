#include "RunData.h"
#include <gtest/gtest.h>

class RunDataTest : public ::testing::Test {};

TEST(RunDataTest, set_and_get_start_time) {
  auto rundata = RunData();
  EXPECT_NO_THROW(rundata.setStartTime(1470901818));
  EXPECT_EQ("2016-08-11T08:50:18", rundata.getStartTime());
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
