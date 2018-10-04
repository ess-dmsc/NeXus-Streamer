#include <gmock/gmock.h>
#include <memory>

#include "../../event_data/include/DetectorSpectrumMapData.h"
#include "MockEventPublisher.h"
#include "NexusPublisher.h"
#include "OptionalArgs.h"

using ::testing::AtLeast;
using ::testing::_;

class FakeFileReader : public FileReader {
  hsize_t getFileSize() override { return 0; };
  uint64_t getTotalEventCount() override { return 3; };
  uint32_t getPeriodNumber() override { return 0; };
  float getProtonCharge(hsize_t frameNumber) override { return 0.002; };

  bool getEventDetIds(std::vector<uint32_t> &detIds,
                      hsize_t frameNumber) override {
    detIds = {0, 1, 2};
    return true;
  };

  bool getEventTofs(std::vector<uint32_t> &tofs, hsize_t frameNumber) override {
    tofs = {0, 1, 2};
    return true;
  };

  size_t getNumberOfFrames() override { return 1; };
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber) override { return 3; };
  uint64_t getFrameTime(hsize_t frameNumber) override { return 0; };
  std::string getInstrumentName() override { return "FAKE"; };
  std::unordered_map<hsize_t, sEEventVector> getSEEventMap() override {
    return {};
  };
  int32_t getNumberOfPeriods() override { return 1; };
  uint64_t getRelativeFrameTimeMilliseconds(hsize_t frameNumber) override {
    return 0;
  };
};

class NexusPublisherTest : public ::testing::Test {
public:
  OptionalArgs createSettings(bool quiet) {
    extern std::string testDataPath;

    OptionalArgs settings;
    settings.broker = "broker_name";
    settings.instrumentName = "unitTest";
    settings.quietMode = quiet;
    settings.filename = testDataPath + "SANS_test_reduced.hdf5";
    settings.detSpecFilename = testDataPath + "spectrum_gastubes_01.dat";
    return settings;
  }

  NexusPublisher createStreamer(const bool quiet) {
    const auto settings = createSettings(quiet);

    auto publisher = std::make_shared<MockEventPublisher>();

    std::shared_ptr<FileReader> fakeFileReader =
        std::make_shared<FakeFileReader>();
    NexusPublisher streamer(publisher, fakeFileReader, settings);
    return streamer;
  }
};

TEST_F(NexusPublisherTest, test_create_streamer) { createStreamer(false); }

TEST_F(NexusPublisherTest, test_create_streamer_quiet) { createStreamer(true); }

TEST_F(NexusPublisherTest, test_create_message_data) {
  auto streamer = createStreamer(true);
  auto eventData = streamer.createMessageData(static_cast<hsize_t>(0));

  std::string rawbuf;
  eventData[0]->getBufferPointer(rawbuf, 0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(rawbuf));
  EXPECT_EQ(3, receivedEventData.getNumberOfEvents());
  EXPECT_FLOAT_EQ(0.002, receivedEventData.getProtonCharge());
  EXPECT_EQ(0, receivedEventData.getPeriod());
}

TEST_F(NexusPublisherTest, test_stream_data) {
  using ::testing::Sequence;

  const auto settings = createSettings(true);

  auto publisher = std::make_shared<MockEventPublisher>();
  publisher->setUp(settings.broker, settings.instrumentName);

  const int numberOfFrames = 1;

  EXPECT_CALL(*publisher.get(), sendEventMessage(_, _)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_, _))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_, _)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(streamer.streamData(1, false));
}

TEST_F(NexusPublisherTest, test_data_is_streamed_in_slow_mode) {
  using ::testing::Sequence;

  const auto settings = createSettings(true);

  auto publisher = std::make_shared<MockEventPublisher>();
  publisher->setUp(settings.broker, settings.instrumentName);

  const int numberOfFrames = 1;

  EXPECT_CALL(*publisher.get(), sendEventMessage(_, _)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_, _))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_, _)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(streamer.streamData(1, true));
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
