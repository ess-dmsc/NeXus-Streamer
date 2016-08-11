#include "EventData.h"
#include <gtest/gtest.h>

class EventDataTest : public ::testing::Test {};

TEST(EventDataTest, get_tof_and_detid) {
  auto events = EventData();
  EXPECT_EQ(0, events.getTof().size());
  EXPECT_EQ(0, events.getDetId().size());
  EXPECT_EQ(0, events.getFrameNumber());
  EXPECT_EQ(0, events.getTotalCounts());
}

TEST(EventDataTest, get_buffer_pointer) {
  auto events = EventData();

  std::vector<int32_t> detIds = {1, 2, 3, 4};
  std::vector<float> tofs = {4, 3, 2, 1};
  uint32_t frameNumber = 2;
  float protonCharge = 0.001142;
  float frameTime = 4.1389;
  int32_t period = 1;

  EXPECT_NO_THROW(events.setDetId(detIds));
  EXPECT_NO_THROW(events.setTof(tofs));
  EXPECT_NO_THROW(events.setFrameNumber(frameNumber));
  EXPECT_NO_THROW(events.setEndOfRun(true));
  EXPECT_NO_THROW(events.setEndOfFrame(true));
  EXPECT_NO_THROW(events.setProtonCharge(protonCharge));
  EXPECT_NO_THROW(events.setFrameTime(frameTime));
  EXPECT_NO_THROW(events.setPeriod(period));

  std::string rawbuf;
  EXPECT_NO_THROW(events.getBufferPointer(rawbuf));

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(4, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(detIds, receivedEventData.getDetId());
  EXPECT_EQ(tofs, receivedEventData.getTof());
  EXPECT_EQ(frameNumber, receivedEventData.getFrameNumber());
  EXPECT_TRUE(receivedEventData.getEndOfRun());
  EXPECT_TRUE(receivedEventData.getEndOfFrame());
  EXPECT_FLOAT_EQ(protonCharge, receivedEventData.getProtonCharge());
  EXPECT_FLOAT_EQ(frameTime, receivedEventData.getFrameTime());
  EXPECT_EQ(period, receivedEventData.getPeriod());
}

TEST(EventDataTest, get_buffer_size) {
  auto events = EventData();

  std::vector<int32_t> detIds = {1, 2, 3, 4};
  std::vector<float> tofs = {4, 3, 2, 1};

  events.setDetId(detIds);
  events.setTof(tofs);

  std::string rawbuf;
  EXPECT_NO_THROW(events.getBufferPointer(rawbuf));
  EXPECT_EQ(148, events.getBufferSize());
}
