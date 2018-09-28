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
                            testDataPath + "spectrum_gastubes_01.dat", quiet);
    return streamer;
  }
};

TEST_F(NexusPublisherTest, test_create_streamer) { createStreamer(false); }

TEST_F(NexusPublisherTest, test_create_streamer_quiet) { createStreamer(true); }

TEST_F(NexusPublisherTest, test_create_message_data) {
  auto streamer = createStreamer(true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(1));

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf, 0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(rawbuf));
  EXPECT_EQ(770, receivedEventData.getNumberOfEvents());
  EXPECT_FLOAT_EQ(0.001105368, receivedEventData.getProtonCharge());
  EXPECT_EQ(0, receivedEventData.getPeriod());
}

TEST_F(NexusPublisherTest, test_stream_data) {
  using ::testing::Sequence;
  extern std::string testDataPath;

  const std::string broker = "broker_name";
  const std::string instrumentName = "unitTest";

  auto publisher = std::make_shared<MockEventPublisher>();

  const int numberOfFrames = 300;

  EXPECT_CALL(*publisher.get(), setUp(broker, instrumentName))
      .Times(AtLeast(1));

  EXPECT_CALL(*publisher.get(), sendEventMessage(_, _)).Times(numberOfFrames);

  EXPECT_CALL(*publisher.get(), sendSampleEnvMessage(_, _)).Times(16);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_, _))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_, _)).Times(1);

  NexusPublisher streamer(publisher, broker, instrumentName,
                          testDataPath + "SANS_test_reduced.hdf5",
                          testDataPath + "spectrum_gastubes_01.dat", true);
  EXPECT_NO_THROW(streamer.streamData(1, false));
}

TEST_F(NexusPublisherTest, test_create_run_message_data) {
  auto streamer = createStreamer(true);
  int runNumber = 3;
  auto runData = streamer.createRunMessageData(runNumber);

  std::string rawbuf;
  runData->getRunStartBufferPointer(rawbuf);

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
  EXPECT_EQ(122888, receivedData.getNumberOfEntries());
}
