#include "SampleEnvironmentEvent.h"
#include "SampleEnvironmentEventDouble.h"
#include "SampleEnvironmentEventInt.h"
#include "SampleEnvironmentEventLong.h"
#include <gtest/gtest.h>

class SampleEnvironmentEventTest : public ::testing::Test {};

TEST(SampleEnvironmentEventTest, create_int_event) {
  auto value = std::numeric_limits<int32_t>::max();
  int64_t runStart = 0;
  EXPECT_NO_THROW(SampleEnvironmentEventInt("TEMP_1", 0.242, value, runStart));
}

TEST(SampleEnvironmentEventTest, get_int_event) {
  auto value = std::numeric_limits<int32_t>::max();
  int64_t runStart = 0;
  auto intEvent = SampleEnvironmentEventInt("TEMP_1", 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(intEvent.getSEEvent(builder));
}

TEST(SampleEnvironmentEventTest, create_long_event) {
  auto value = std::numeric_limits<int64_t>::max();
  int64_t runStart = 0;
  EXPECT_NO_THROW(SampleEnvironmentEventLong("TEMP_1", 0.242, value, runStart));
}

TEST(SampleEnvironmentEventTest, get_long_event) {
  auto value = std::numeric_limits<int64_t>::max();
  int64_t runStart = 0;
  auto longEvent = SampleEnvironmentEventLong("TEMP_1", 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(longEvent.getSEEvent(builder));
}

TEST(SampleEnvironmentEventTest, create_double_event) {
  double value = 42.12;
  int64_t runStart = 0;
  EXPECT_NO_THROW(SampleEnvironmentEventDouble("TEMP_1", 0.242, value, runStart));
}

TEST(SampleEnvironmentEventTest, get_double_event) {
  double value = 42.12;
  int64_t runStart = 0;
  auto doubleEvent = SampleEnvironmentEventDouble("TEMP_1", 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(doubleEvent.getSEEvent(builder));
}
