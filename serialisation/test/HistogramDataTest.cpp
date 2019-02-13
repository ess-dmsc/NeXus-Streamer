#include "HistogramData.h"
#include "../../core/include/Message.h"
#include <gtest/gtest.h>

class HistogramDataTest : public ::testing::Test {};

TEST(HistogramDataTest, expect_createHistogramMessage_to_return_a_message_of_nonzero_size) {
  auto message = createHistogramMessage({1, 2, 3}, {1.0, 2.0, 3.0});
  EXPECT_GT(message.size(), 0);
}
