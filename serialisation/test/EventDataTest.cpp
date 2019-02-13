#include "EventData.h"
#include "../../core/include/Message.h"
#include <ev42_events_generated.h>
#include <gtest/gtest.h>

class EventDataTest : public ::testing::Test {};

TEST(EventDataTest, get_tof_and_detid) {
  auto events = EventData();
  EXPECT_EQ(0, events.getTof().size());
  EXPECT_EQ(0, events.getDetId().size());
  EXPECT_EQ(0, events.getTotalCounts());
}

TEST(EventDataTest, get_buffer_pointer) {
  auto events = EventData();

  std::vector<uint32_t> detIds = {1, 2, 3, 4};
  std::vector<uint32_t> tofs = {4, 3, 2, 1};
  float protonCharge = 0.001142;
  float frameTime = 41389;
  int32_t period = 1;

  EXPECT_NO_THROW(events.setDetId(detIds));
  EXPECT_NO_THROW(events.setTof(tofs));
  EXPECT_NO_THROW(events.setProtonCharge(protonCharge));
  EXPECT_NO_THROW(events.setFrameTime(frameTime));
  EXPECT_NO_THROW(events.setPeriod(period));

  auto buffer = events.getBuffer(0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(buffer.data())));
  EXPECT_EQ(4, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(detIds, receivedEventData.getDetId());
  EXPECT_EQ(tofs, receivedEventData.getTof());
  EXPECT_FLOAT_EQ(protonCharge, receivedEventData.getProtonCharge());
  EXPECT_FLOAT_EQ(frameTime, receivedEventData.getFrameTime());
  EXPECT_EQ(period, receivedEventData.getPeriod());
}

TEST(EventDataTest, get_buffer_size) {
  auto events = EventData();

  std::vector<uint32_t> detIds = {1, 2, 3, 4};
  std::vector<uint32_t> tofs = {4, 3, 2, 1};

  events.setDetId(detIds);
  events.setTof(tofs);

  auto buffer = events.getBuffer(0);
  EXPECT_TRUE(buffer.size() > 0);
}

TEST(EventDataTest, check_buffer_includes_file_identifier) {
  auto events = EventData();

  std::vector<uint32_t> detIds = {1, 2, 3, 4};
  std::vector<uint32_t> tofs = {4, 3, 2, 1};

  events.setDetId(detIds);
  events.setTof(tofs);

  auto buffer = events.getBuffer(0);

  auto eventIdentifier = EventMessageIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(buffer.data()), eventIdentifier));
}
