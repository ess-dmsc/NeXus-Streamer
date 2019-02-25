#include <gmock/gmock.h>
#include <memory>

#include "../../core/include/EventDataFrame.h"
#include "../../core/include/HistogramFrame.h"
#include "../../serialisation/include/DetectorSpectrumMapData.h"
#include "../../serialisation/include/EventData.h"
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

  std::vector<HistogramFrame> getHistoData() override {
    std::vector<int32_t> detectorCounts{1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<size_t> countsShape{1, 3, 3};
    std::vector<float> tofBinEdges{1.0, 2.0, 3.0};
    std::vector<int32_t> detectorIds{1, 2, 3};
    std::vector<HistogramFrame> histoData;
    histoData.emplace_back(detectorCounts, countsShape, tofBinEdges,
                           detectorIds);
    return histoData;
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
  uint32_t getRunDurationMs() override { return 100; };
};

class NexusPublisherTest : public ::testing::Test {
public:
  OptionalArgs createSettings(bool quiet,
                              std::pair<int32_t, int32_t> minMaxDetNum = {0, 0},
                              bool slow = false) {
    extern std::string testDataPath;

    OptionalArgs settings;
    settings.broker = "broker_name";
    settings.instrumentName = "unitTest";
    settings.quietMode = quiet;
    settings.filename = testDataPath + "SANS_test_reduced.hdf5";
    settings.detSpecFilename = testDataPath + "spectrum_gastubes_01.dat";
    settings.histogramUpdatePeriodMs = 50;
    settings.minMaxDetectorNums = minMaxDetNum;
    settings.slow = slow;
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
  EXPECT_CALL(*publisher.get(), sendHistogramMessage(_)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(streamer.streamData(1, settings));
}

TEST_F(NexusPublisherTest, test_det_spec_not_sent_when_pair_is_specified) {
  using ::testing::Sequence;

  std::pair<int32_t, int32_t> minMaxDetectorNum = {0, 2};
  const auto settings = createSettings(true, minMaxDetectorNum);

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
  EXPECT_NO_THROW(streamer.streamData(1, settings));
}

TEST_F(NexusPublisherTest, test_data_is_streamed_in_slow_mode) {
  using ::testing::Sequence;

  bool slowMode = true;
  const auto settings = createSettings(true, {0, 0}, slowMode);

  auto publisher = std::make_shared<MockEventPublisher>();
  publisher->setUp(settings.broker, settings.instrumentName);

  const int numberOfFrames = 1;

  EXPECT_CALL(*publisher.get(), sendEventMessage(_)).Times(numberOfFrames);
  EXPECT_CALL(*publisher.get(), sendRunMessage(_))
      .Times(2); // Start and stop messages
  EXPECT_CALL(*publisher.get(), sendHistogramMessage(_)).Times(2);
  EXPECT_CALL(*publisher.get(), sendDetSpecMessage(_)).Times(1);

  std::shared_ptr<FileReader> fakeFileReader =
      std::make_shared<FakeFileReader>();
  NexusPublisher streamer(publisher, fakeFileReader, settings);
  EXPECT_NO_THROW(streamer.streamData(1, settings));
}
