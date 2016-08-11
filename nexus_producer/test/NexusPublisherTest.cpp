#include <gmock/gmock.h>
#include <memory>

#include "MockEventPublisher.h"
#include "NexusPublisher.h"

using ::testing::AtLeast;
using ::testing::_;

class NexusPublisherTest : public ::testing::Test {};

TEST(NexusPublisherTest, test_create_streamer) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();
  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", false);
}

TEST(NexusPublisherTest, test_create_streamer_quiet) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();
  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", true);
}

TEST(NexusPublisherTest, test_create_message_data) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(1), 1);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(770, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(1, receivedEventData.getFrameNumber());
  EXPECT_FLOAT_EQ(0.001105368, receivedEventData.getProtonCharge());
  EXPECT_EQ(1, receivedEventData.getPeriod());
  EXPECT_FLOAT_EQ(3.0399999618530273, receivedEventData.getFrameTime());
}

TEST(NexusPublisherTest, test_create_message_data_3_message_per_frame) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";
  int frameNumber = 1;

  auto publisher = std::make_shared<MockEventPublisher>();

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(frameNumber), 3);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // First message should have ceil(770/3) events
  EXPECT_EQ(257, receivedEventData.getNumberOfEvents());
  EXPECT_EQ(frameNumber, receivedEventData.getFrameNumber());
  // and should not be the last message in the frame or in the run
  EXPECT_FALSE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());

  eventData[2]->getBufferPointer(rawbuf);
  EXPECT_TRUE(receivedEventData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // Last message in frame should have remaining 256 events
  EXPECT_EQ(256, receivedEventData.getNumberOfEvents());
  // and should be the last message in the frame but not in the run
  EXPECT_TRUE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());
}

TEST(NexusPublisherTest, test_create_message_data_3_message_per_frame_end_of_run) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";
  int frameNumber = 299;

  auto publisher = std::make_shared<MockEventPublisher>();

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(frameNumber), 3);

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf);

  auto receivedData = RunData();
  // Should return false as this is not run data
  EXPECT_FALSE(receivedData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // First message of frame
  EXPECT_EQ(frameNumber, receivedEventData.getFrameNumber());
  // should not be the last message in the frame or in the run
  EXPECT_FALSE(receivedEventData.getEndOfFrame());
  EXPECT_FALSE(receivedEventData.getEndOfRun());

  eventData[2]->getBufferPointer(rawbuf);
  EXPECT_TRUE(receivedEventData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  // Last message should be the last message in the frame and in the run
  EXPECT_TRUE(receivedEventData.getEndOfFrame());
  EXPECT_TRUE(receivedEventData.getEndOfRun());
}

TEST(NexusPublisherTest, test_stream_data) {

  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();

  const int numberOfFrames = 300;
  const int messagesPerFrame = 1;

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(1);
  EXPECT_CALL(*publisher.get(), sendMessage(_, _))
      .Times(numberOfFrames * messagesPerFrame);

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", false);
  EXPECT_NO_THROW(streamer.streamData(messagesPerFrame));
}

TEST(NexusPublisherTest, test_stream_data_multiple_messages_per_frame) {

  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();

  const int numberOfFrames = 300;
  const int messagesPerFrame = 10;

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(1);
  EXPECT_CALL(*publisher.get(), sendMessage(_, _))
      .Times(numberOfFrames * messagesPerFrame);

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", false);
  EXPECT_NO_THROW(streamer.streamData(messagesPerFrame));
}

TEST(NexusPublisherTest, test_create_run_message_data) {
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string topic = "topic_name";

  auto publisher = std::make_shared<MockEventPublisher>();

  EXPECT_CALL(*publisher.get(), setUp(broker, topic)).Times(AtLeast(1));

  NexusPublisher streamer(publisher, broker, topic,
                          testDataPath + "SANS_test_reduced.hdf5", true);
  int runNumber = 3;
  auto runData = streamer.createRunMessageData(runNumber);

  std::string rawbuf;
  runData->getBufferPointer(rawbuf);

  auto receivedData = EventData();
  // Should return false as this is not event data
  EXPECT_FALSE(receivedData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(runNumber, receivedRunData.getRunNumber());
}
