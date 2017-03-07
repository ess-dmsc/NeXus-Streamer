#include <gmock/gmock.h>
#include <memory>

#include "../../event_data/include/DetectorSpectrumMapData.h"
#include "MockEventPublisher.h"
#include "NexusPublisher.h"

using ::testing::AtLeast;
using ::testing::_;

class NexusPublisherTest : public ::testing::Test {
public:
  NexusPublisher createStreamer(bool quiet) {
    extern std::string testDataPath;

    const std::string broker = "broker_name";
    const std::string instrumentName = "unitTest";

    auto publisher = std::make_shared<MockEventPublisher>();
    EXPECT_CALL(*publisher.get(), setUp(broker, instrumentName))
        .Times(AtLeast(1));

    NexusPublisher streamer(publisher, broker, instrumentName,
                            testDataPath + "SANS_test_reduced.hdf5",
                            testDataPath + "spectrum_gastubes_01.dat", quiet,
                            false);
    return streamer;
  }
};

TEST_F(NexusPublisherTest, test_create_streamer) { createStreamer(false); }

TEST_F(NexusPublisherTest, test_create_streamer_quiet) { createStreamer(true); }

TEST_F(NexusPublisherTest, test_create_message_data) {
  auto streamer = createStreamer(true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(1), 1);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf, 0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(770, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(1, receivedEventData.getFrameNumber());
  EXPECT_FLOAT_EQ(0.001105368, receivedEventData.getProtonCharge());
  EXPECT_EQ(0, receivedEventData.getPeriod());
  EXPECT_FLOAT_EQ(3.0399999618530273, receivedEventData.getFrameTime());
}

TEST_F(NexusPublisherTest, test_create_message_data_3_message_per_frame) {
  auto streamer = createStreamer(true);
  int frameNumber = 1;
  auto eventData =
      streamer.createMessageData(static_cast<hsize_t>(frameNumber), 3);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf, 0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // First message should have ceil(770/3) events
  EXPECT_EQ(257, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(frameNumber, receivedEventData.getFrameNumber());
  // and should not be the last message in the frame or in the run
  EXPECT_FALSE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());

  eventData[2]->getBufferPointer(rawbuf, 1);
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // Last message in frame should have remaining 256 events
  EXPECT_EQ(256, receivedEventData.getNumberOfEvents());
  // and should be the last message in the frame but not in the run
  EXPECT_TRUE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());
}

TEST_F(NexusPublisherTest,
       test_create_message_data_3_message_per_frame_end_of_run) {
  auto streamer = createStreamer(true);
  int frameNumber = 299;
  auto eventData =
      streamer.createMessageData(static_cast<hsize_t>(frameNumber), 3);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf, 0);

  auto receivedData = RunData();
  // Should return false as this is not run data
  EXPECT_FALSE(receivedData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // First message of frame
  EXPECT_EQ(frameNumber, receivedEventData.getFrameNumber());
  // should not be the last message in the frame or in the run
  EXPECT_FALSE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());

  eventData[2]->getBufferPointer(rawbuf, 0);
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // Last message should be the last message in the frame and in the run
  EXPECT_TRUE(receivedEventData.getEndOfFrame());
  EXPECT_TRUE(receivedEventData.getEndOfRun());
}

MATCHER_P(CheckMessageID, messageID, "") {
  auto buf = reinterpret_cast<const uint8_t *>(arg);
  auto messageData = ISISStream::GetEventMessage(buf);
  return (messageID == messageData->id());
}

TEST_F(NexusPublisherTest, test_stream_data) {
  using ::testing::Sequence;
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string instrumentName = "unitTest";

  auto publisher = std::make_shared<MockEventPublisher>();

  const int numberOfFrames = 300;
  const int maxEventsPerFramePart = 1000000;

  EXPECT_CALL(*publisher.get(), setUp(broker, instrumentName))
      .Times(AtLeast(1));

  // test that messages have sequential id numbers
  Sequence s1;
  for (uint64_t messageID = 0; messageID <= numberOfFrames; messageID++) {
    EXPECT_CALL(*publisher.get(),
                sendEventMessage(CheckMessageID(messageID), _))
        .InSequence(s1);
  }

  EXPECT_CALL(*publisher.get(), sendRunMessage(_, _)).Times(1);
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_, _)).Times(1);
  EXPECT_CALL(*publisher.get(), getCurrentOffset()).Times(1);

  NexusPublisher streamer(publisher, broker, instrumentName,
                          testDataPath + "SANS_test_reduced.hdf5",
                          testDataPath + "spectrum_gastubes_01.dat", true,
                          false);
  EXPECT_NO_THROW(streamer.streamData(maxEventsPerFramePart, 1, false));
}

TEST_F(NexusPublisherTest, test_stream_data_multiple_messages_per_frame) {

  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string instrumentName = "unitTest";

  auto publisher = std::make_shared<MockEventPublisher>();

  const int maxEventsPerFramePart = 200;

  EXPECT_CALL(*publisher.get(), setUp(broker, instrumentName))
      .Times(AtLeast(1));
  EXPECT_CALL(*publisher.get(), sendEventMessage(_, _)).Times(1292);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_, _)).Times(1);
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_, _)).Times(1);
  EXPECT_CALL(*publisher.get(), getCurrentOffset()).Times(1);

  NexusPublisher streamer(publisher, broker, instrumentName,
                          testDataPath + "SANS_test_reduced.hdf5",
                          testDataPath + "spectrum_gastubes_01.dat", true,
                          true);
  EXPECT_NO_THROW(streamer.streamData(maxEventsPerFramePart, 1, false));
}

TEST_F(NexusPublisherTest, test_create_run_message_data) {
  auto streamer = createStreamer(true);
  int runNumber = 3;
  auto runData = streamer.createRunMessageData(runNumber);

  std::string rawbuf;
  runData->getEventBufferPointer(rawbuf, 0);

  auto receivedData = EventData();
  // Should return false as this is not event data
  EXPECT_FALSE(receivedData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(runNumber, receivedRunData.getRunNumber());
}

TEST_F(NexusPublisherTest, test_create_det_spec_map_message_data) {
  auto streamer = createStreamer(true);
  auto detSpecMap = streamer.createDetSpecMessageData();

  std::string rawbuf;
  detSpecMap->getBufferPointer(rawbuf);

  auto receivedData = DetectorSpectrumMapData();
  EXPECT_NO_THROW(receivedData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(245768, receivedData.getNumberOfEntries());
}
