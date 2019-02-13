#include <gmock/gmock.h>
#include <memory>

#include "../../serialisation/include/DetectorSpectrumMapData.h"
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

  std::vector<EventDataFrame> getEventData(hsize_t frameNumber) override {
    std::vector<EventDataFrame> eventData{EventDataFrame({0, 1, 2}, {0, 1, 2})};
    return eventData;
  }

  std::vector<HistogramData> getHistoData() override {
    HistogramData histoData{{1, 2, 3}, {1.0, 2.0, 3.0}};
    return {histoData};
  }

  size_t getNumberOfFrames() override { return 1; };
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber,
                                   size_t eventGroupNumber) override {
    return 3;
  };
  uint64_t getFrameTime(hsize_t frameNumber) override { return 0; };
  std::string getInstrumentName() override { return "FAKE"; };
  std::unordered_map<hsize_t, sEEventVector> getSEEventMap() override {
    return {};
  };
  int32_t getNumberOfPeriods() override { return 1; };
  uint64_t getRelativeFrameTimeMilliseconds(hsize_t frameNumber) override {
    return 0;
  };
  bool isISISFile() override { return true; };
  uint64_t getTotalEventsInGroup(size_t eventGroupNumber) override {
    return 3;
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

  auto buffer = eventData[0].getBuffer(0);

  auto receivedEventData = EventData();
  EXPECT_TRUE(receivedEventData.decodeMessage(
      reinterpret_cast<uint8_t *>(buffer.data())));
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

  EXPECT_CALL(*publisher.get(), sendEventMessage(_)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(
      streamer.streamData(1, false, std::make_pair<int32_t, int32_t>(0, 0)));
}

TEST_F(NexusPublisherTest, test_det_spec_not_sent_when_pair_is_empty) {
  using ::testing::Sequence;

  const auto settings = createSettings(true);

  auto publisher = std::make_shared<MockEventPublisher>();
  publisher->setUp(settings.broker, settings.instrumentName);

  const int numberOfFrames = 1;

  EXPECT_CALL(*publisher.get(), sendEventMessage(_)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_)).Times(0);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(
      streamer.streamData(1, false, std::make_pair<int32_t, int32_t>(1, 2)));
}

TEST_F(NexusPublisherTest, test_data_is_streamed_in_slow_mode) {
  using ::testing::Sequence;

  const auto settings = createSettings(true);

  auto publisher = std::make_shared<MockEventPublisher>();
  publisher->setUp(settings.broker, settings.instrumentName);

  const int numberOfFrames = 1;

  EXPECT_CALL(*publisher.get(), sendEventMessage(_)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(
      streamer.streamData(1, false, std::make_pair<int32_t, int32_t>(0, 0)));
}

TEST_F(NexusPublisherTest, test_create_run_message_data) {
  auto streamer = createStreamer(true);
  int runNumber = 3;
  auto runData = streamer.createRunMessageData(runNumber);

  auto buffer = runData->getRunStartBuffer();

  auto receivedRunData = RunData();
  EXPECT_TRUE(receivedRunData.decodeMessage(
      reinterpret_cast<const uint8_t *>(buffer.data())));
  EXPECT_EQ(runNumber, receivedRunData.getRunNumber());
}

TEST_F(NexusPublisherTest, test_create_det_spec_map_message_data) {
  auto streamer = createStreamer(true);
  auto detSpecMap = streamer.createDetSpecMessageData();

  auto buffer = detSpecMap->getBuffer();

  auto receivedData = DetectorSpectrumMapData();
  EXPECT_NO_THROW(receivedData.decodeMessage(
      reinterpret_cast<const uint8_t *>(buffer.data())));
  EXPECT_EQ(122888, receivedData.getNumberOfEntries());
}
