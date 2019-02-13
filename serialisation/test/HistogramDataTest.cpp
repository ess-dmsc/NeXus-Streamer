#include "../../core/include/Message.h"
#include "HistogramData.h"
#include "hs00_event_histogram_generated.h"
#include <gtest/gtest.h>

class HistogramDataTest : public ::testing::Test {};

TEST(
    HistogramDataTest,
    expect_createHistogramMessage_to_return_a_message_with_histogram_schema_identifier) {
  auto message = createHistogramMessage({1, 2, 3}, {1, 1, 3}, {1.0, 3.0}, 0);

  EXPECT_GT(message.size(), 8) << "Expected message size to at least be large "
                                  "enough to contain the file identifier";

  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(message.data()),
      EventHistogramIdentifier()));
}
