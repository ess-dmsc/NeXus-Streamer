#include "../../core/include/Message.h"
#include "HistogramData.h"
#include <gtest/gtest.h>

class HistogramDataTest : public ::testing::Test {};

TEST(HistogramDataTest,
     expect_createHistogramMessage_to_return_a_message_with_hs00_identifier) {
  auto message = createHistogramMessage({1, 2, 3}, {1, 1, 3}, {1.0, 3.0}, 0);

  EXPECT_GT(message.size(), 8) << "Expected message size to at least be large "
                                  "enough to contain the file identifier";

  // Flatbuffer file identifier is in chars 4 to 7 of the data buffer
  EXPECT_EQ(message.data()[4], 'h');
  EXPECT_EQ(message.data()[5], 's');
  EXPECT_EQ(message.data()[6], '0');
  EXPECT_EQ(message.data()[7], '0');
}
