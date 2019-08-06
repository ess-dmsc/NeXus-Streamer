#include "UnitConversion.h"
#include <gtest/gtest.h>

class UnitConversionTest : public ::testing::Test {};

TEST(UnitConversionTest,
     test_seconds_to_nanoseconds_converts_1_seconds_to_1e9_nanoseconds) {
  // Test with integer value (as double type)
  double inputSeconds = 1.0;
  auto outputNanoseconds = secondsToNanoseconds(inputSeconds);
  ASSERT_EQ(outputNanoseconds, 1000000000);
}

TEST(UnitConversionTest,
     test_seconds_to_nanoseconds_converts_1_23_seconds_to_123e7_nanoseconds) {
  // Test with non-integer value
  double inputSeconds = 1.23;
  auto outputNanoseconds = secondsToNanoseconds(inputSeconds);
  ASSERT_EQ(outputNanoseconds, 1230000000);
}

TEST(UnitConversionTest,
     test_seconds_to_nanoseconds_also_works_when_given_vector_input) {
  std::vector<double> inputSeconds = {1.0, 1.23};
  auto outputNanoseconds = secondsToNanoseconds(inputSeconds);
  ASSERT_EQ(outputNanoseconds[0], 1000000000);
  ASSERT_EQ(outputNanoseconds[1], 1230000000);
}

TEST(UnitConversionTest,
     test_seconds_to_milliseconds_converts_1_seconds_to_1000_milliseconds) {
  // Test with integer value (as double type)
  double inputSeconds = 1.0;
  auto outputMilliseconds = secondsToMilliseconds(inputSeconds);
  ASSERT_EQ(outputMilliseconds, 1000);
}

TEST(UnitConversionTest,
     test_seconds_to_milliseconds_converts_1_23_seconds_to_1230_milliseconds) {
  // Test with non-integer value
  double inputSeconds = 1.23;
  auto outputMilliseconds = secondsToMilliseconds(inputSeconds);
  ASSERT_EQ(outputMilliseconds, 1230);
}

TEST(
    UnitConversionTest,
    test_nanoseconds_to_milliseconds_converts_1000000_nanoseconds_to_1_milliseconds) {
  // Test with integer value (as double type)
  uint64_t inputNanoseconds = 1000000;
  auto outputMilliseconds = nanosecondsToMilliseconds(inputNanoseconds);
  ASSERT_EQ(outputMilliseconds, 1);
}

TEST(
    UnitConversionTest,
    test_nanoseconds_to_milliseconds_converts_and_rounds_1230000_nanoseconds_to_1_milliseconds) {
  // Test with non-integer value
  uint64_t inputNanoseconds = 1230000;
  auto outputMilliseconds = nanosecondsToMilliseconds(inputNanoseconds);
  ASSERT_EQ(outputMilliseconds, 1);
}
