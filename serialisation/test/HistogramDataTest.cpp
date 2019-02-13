#include "../../core/include/Message.h"
#include "HistogramData.h"
#include "hs00_event_histogram_generated.h"
#include <gtest/gtest.h>

::testing::AssertionResult
AllElementsInVectorAreNear(const std::vector<float> &a,
                           const std::vector<float> &b, float delta) {

  if (a.size() != b.size()) {
    return ::testing::AssertionFailure() << "Vectors are different lengths";
  }

  for (size_t i = 0; i < a.size(); ++i) {
    if (a[i] < (b[i] - delta) || a[i] > (b[i] + delta)) {
      return ::testing::AssertionFailure() << "Vectors differ by more than "
                                           << delta;
    }
  }

  return ::testing::AssertionSuccess();
}

class HistogramDataTest : public ::testing::Test {};

TEST(
    HistogramDataTest,
    createHistogramMessage_returns_a_message_with_histogram_schema_identifier) {
  auto message = createHistogramMessage({1, 2, 3}, {1, 1, 3}, {1.0, 3.0}, 0);

  EXPECT_GT(message.size(), 8) << "Expected message size to at least be large "
                                  "enough to contain the file identifier";

  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(message.data()),
      EventHistogramIdentifier()));
}

TEST(
    HistogramDataTest,
    deserialising_message_created_by_createHistogramMessage_yeilds_original_input) {

  std::vector<int32_t> countsInput{1, 2, 3};
  std::vector<size_t> countsShapeInput{1, 1, 3};
  std::vector<float> timeOfFlightInput{1.0, 3.0};
  uint64_t timestampUnixInput = 0;

  auto message = createHistogramMessage(countsInput, countsShapeInput,
                                        timeOfFlightInput, timestampUnixInput);

  std::vector<int32_t> countsOutput;
  std::vector<size_t> countsShapeOutput;
  std::vector<float> timeOfFlightOutput;
  uint64_t timestampUnixOutput;

  deserialiseHistogramMessage(message, countsOutput, countsShapeOutput,
                              timeOfFlightOutput, timestampUnixOutput);

  EXPECT_EQ(countsInput, countsOutput);
  EXPECT_EQ(countsShapeInput, countsShapeOutput);
  EXPECT_TRUE(
      AllElementsInVectorAreNear(timeOfFlightInput, timeOfFlightOutput, 0.01));
  EXPECT_EQ(timestampUnixInput, timestampUnixOutput);
}
