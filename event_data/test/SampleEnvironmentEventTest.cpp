#include "SampleEnvironmentEvent.h"
#include "SampleEnvironmentEventInt.h"
#include <gtest/gtest.h>

class SampleEnvironmentEventTest : public ::testing::Test {};

TEST(SampleEnvironmentEventTest, create_int_event) {
  int32_t value = 42;
  EXPECT_NO_THROW(SampleEnvironmentEventInt("TEMP_1", 0.242, value));
}
