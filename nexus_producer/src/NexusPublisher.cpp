#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "../../core/include/EventDataFrame.h"
#include "../../core/include/HistogramFrame.h"
#include "../../serialisation/include/DetectorSpectrumMapData.h"
#include "../../serialisation/include/DetectorSpectrumMapData.h"
#include "../../serialisation/include/EventData.h"
#include "../../serialisation/include/HistogramData.h"
#include "../../serialisation/include/RunData.h"
#include "NexusPublisher.h"
#include "Timer.h"

namespace {

int64_t getTimeNowInNanoseconds() {
  auto now = std::chrono::system_clock::now();
  auto now_epoch = now.time_since_epoch();
  auto now_epoch_nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now_epoch).count();
  return now_epoch_nanoseconds;
}

void createAndSendHistogramMessage(
    const std::vector<HistogramFrame> &histograms,
    const std::shared_ptr<Publisher> &publisher) {
  // One histogram per NXdata group in the file
  for (const auto &histogram : histograms) {
    auto message = createHistogramMessage(
        histogram, static_cast<uint64_t>(getTimeNowInNanoseconds()));
    publisher->sendHistogramMessage(message);
  }
}
}

/**
 * Create an object responsible for the main business logic of the software. It
 * manages reading data from the NeXus file and publishing it to the data
 * stream.
 *
 * @param publisher - the publisher which provides methods to publish the data
 * to a data stream
 * @param brokerAddress - the IP or hostname of the broker for the data stream
 * @param streamName - the name of the data stream, called the topic in the case
 * of a Kafka publisher
 * @param filename - the full path of a NeXus file
 * @return - a NeXusPublisher object, call streamData() on it to start streaming
 * data
 */
NexusPublisher::NexusPublisher(std::shared_ptr<Publisher> publisher,
                               std::shared_ptr<FileReader> fileReader,
                               const OptionalArgs &settings)
    : m_publisher(std::move(publisher)), m_fileReader(std::move(fileReader)),
      m_quietMode(settings.quietMode),
      m_detSpecMapFilename(settings.detSpecFilename) {

  m_sEEventMap = m_fileReader->getSEEventMap();
}

/**
 * For a given frame number, reads the data from file and stores them in
 * messagesPerFrame EventData objects
 *
 * @param frameNumber - the number of the frame for which to construct a message
 * @return - an object containing the data from the specified frame
 */
std::vector<EventData> NexusPublisher::createMessageData(hsize_t frameNumber) {
  std::vector<EventData> eventDataVector;

  auto protonCharge = m_fileReader->getProtonCharge(frameNumber);
  auto period = m_fileReader->getPeriodNumber();
  auto frameTime = m_fileReader->getFrameTime(frameNumber);

  auto eventDataFramesFromFile = m_fileReader->getEventData(frameNumber);

  for (auto const &eventDataFrame : eventDataFramesFromFile) {
    auto eventData = EventData();
    eventData.setProtonCharge(protonCharge);
    eventData.setPeriod(period);
    eventData.setFrameTime(frameTime);
    eventData.setDetId(eventDataFrame.detectorIDs);
    eventData.setTof(eventDataFrame.timeOfFlights);
    eventData.setTotalCounts(m_fileReader->getTotalEventCount());

    eventDataVector.push_back(eventData);
  }

  return eventDataVector;
}

/**
 * Create runData to send from information in the file
 *
 * @param runNumber - number identifying the current run
 * @return runData
 */
RunData NexusPublisher::createRunMessageData(int runNumber) {
  auto runData = RunData();
  runData.setNumberOfPeriods(m_fileReader->getNumberOfPeriods());
  runData.setInstrumentName(m_fileReader->getInstrumentName());
  runData.setRunID(std::to_string(runNumber));
  runData.setStartTime(static_cast<uint64_t>(getTimeNowInNanoseconds()));
  return runData;
}

std::unique_ptr<Timer> NexusPublisher::publishHistogramBatch(
    const std::vector<HistogramFrame> &histograms,
    uint32_t histogramUpdatePeriodMs, int32_t numberOfTimerIterations) {
  std::unique_ptr<Timer> histogramPublishingTimer;
  if (!histograms.empty()) {
    auto Interval = std::chrono::milliseconds(histogramUpdatePeriodMs);
    std::shared_ptr<Sleeper> IntervalSleeper = std::make_shared<RealSleeper>();
    histogramPublishingTimer = std::make_unique<Timer>(
        Interval, IntervalSleeper, numberOfTimerIterations);
    histogramPublishingTimer->addCallback(
        [ histograms, &publisher = this->m_publisher ]() {
          createAndSendHistogramMessage(histograms, publisher);
        });
    histogramPublishingTimer->start();
  }
  return histogramPublishingTimer;
}

/**
 * Start streaming all the data from the file
 */
void NexusPublisher::streamData(int runNumber, const OptionalArgs &settings) {
  // frame numbers run from 0 to numberOfFrames-1
  int64_t totalBytesSent = 0;
  const auto numberOfFrames = m_fileReader->getNumberOfFrames();

  totalBytesSent += createAndSendRunMessage(runNumber);
  if (settings.minMaxDetectorNums.first == 0 &&
      settings.minMaxDetectorNums.second == 0) {
    totalBytesSent += createAndSendDetSpecMessage();
  }
  std::unique_ptr<Timer> histogramStreamer = streamHistogramData(settings);

  uint64_t lastFrameTime = 0;
  for (size_t frameNumber = 0; frameNumber < numberOfFrames; frameNumber++) {
    // Publish messages at approx real message rate
    if (settings.slow) {
      auto frameTime =
          m_fileReader->getRelativeFrameTimeMilliseconds(frameNumber);
      auto frameDuration = frameTime - lastFrameTime;
      std::this_thread::sleep_for(std::chrono::milliseconds(frameDuration));
      lastFrameTime = frameTime;
    }

    totalBytesSent += createAndSendMessage(frameNumber);
    createAndSendSampleEnvMessages(frameNumber);
    reportProgress(static_cast<float>(frameNumber) /
                   static_cast<float>(numberOfFrames));
  }

  if (histogramStreamer != nullptr) {
    histogramStreamer->triggerStop();
    histogramStreamer->waitForStop();
  }

  totalBytesSent += createAndSendRunStopMessage();
  reportProgress(1.0);
  std::cout << std::endl;

  m_logger->info("Frames sent: {}, Bytes sent: {}",
                 m_fileReader->getNumberOfFrames(), totalBytesSent);
}

std::unique_ptr<Timer>
NexusPublisher::streamHistogramData(const OptionalArgs &settings) {
  std::unique_ptr<Timer> histogramStreamer;
  if (!m_fileReader->hasHistogramData() ||
      settings.histogramUpdatePeriodMs == 0) {
    return histogramStreamer;
  }

  auto runDurationMs = m_fileReader->getRunDurationMs();

  int32_t numberOfHistogramUpdates =
      runDurationMs / settings.histogramUpdatePeriodMs;

  // We want to send at least one batch of histogram messages
  numberOfHistogramUpdates = std::max(1, numberOfHistogramUpdates);

  auto histograms = m_fileReader->getHistoData();

  // If the duration of the run is longer/similar to the specified update period
  // for histogram data, or if slow mode was not selected then we are
  // only going to send one batch of messages. So let's go ahead and do that.
  if (numberOfHistogramUpdates == 1 || !settings.slow) {
    createAndSendHistogramMessage(histograms, m_publisher);

    // Otherwise we'll divide up the original histogram and to publish in
    // batches
    // over the duration of the run
  } else {

    // We'll populate this copy of the histograms with data to send in the first
    // batch of messages
    auto firstHistograms = histograms;

    for (size_t histogramIndex = 0; histogramIndex < histograms.size();
         ++histogramIndex) {

      for (size_t countIndex = 0;
           countIndex < histograms[histogramIndex].counts.size();
           ++countIndex) {
        // The first batch of messages is different from the others as it will
        // include any remainders from dividing the counts amongst the messages
        // Thus summing all messages will result in the original histogram from
        // file
        firstHistograms[histogramIndex].counts[countIndex] =
            (histograms[histogramIndex].counts[countIndex] %
             numberOfHistogramUpdates) +
            (histograms[histogramIndex].counts[countIndex] /
             numberOfHistogramUpdates);
        histograms[histogramIndex].counts[countIndex] =
            histograms[histogramIndex].counts[countIndex] /
            numberOfHistogramUpdates;
      }
    }

    // Send the first batch of histograms and start a timer in a separate thread
    // which will periodically publish each future batch
    createAndSendHistogramMessage(firstHistograms, m_publisher);
    --numberOfHistogramUpdates; // -1 for the first batch that we've already
                                // sent
    histogramStreamer = publishHistogramBatch(
        histograms, settings.histogramUpdatePeriodMs, numberOfHistogramUpdates);
  }
  return histogramStreamer;
}

/**
 * Using Google Flatbuffers, create a message for the specified frame and send
 * it
 *
 * @param rawbuf - a buffer for the message
 * @param frameNumber - the number of the frame for which data will be sent
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendMessage(size_t frameNumber) {
  auto messageData = createMessageData(frameNumber);
  size_t dataSize = 0;
  for (auto &message : messageData) {
    auto buffer = message.getBuffer(m_messageID);
    m_publisher->sendEventMessage(buffer);
    ++m_messageID;
    dataSize += buffer.size();
  }
  return dataSize;
}

/**
 * Create a flatbuffer payload for sample environment log messages
 *
 * @param sampleEnvBuf - a buffer for the message
 * @param frameNumber - the number of the frame for which data will be sent
 */
void NexusPublisher::createAndSendSampleEnvMessages(size_t frameNumber) {
  for (const auto &sEEvent : m_sEEventMap[frameNumber]) {
    auto buffer = sEEvent->getBuffer();
    m_publisher->sendSampleEnvMessage(buffer);
  }
}

/**
 * Create a message containing run metadata and send it
 *
 * @param rawbuf - a buffer for the message
 * @param runNumber - integer to identify the run
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendRunMessage(int runNumber) {
  auto messageData = createRunMessageData(runNumber);
  auto buffer = messageData.getRunStartBuffer();
  m_publisher->sendRunMessage(buffer);
  m_logger->info("Publishing new run: {}", messageData.runInfo());
  return buffer.size();
}

/**
 * Signal that the end of the run has been reached
 *
 * @param rawbuf - a buffer for the message
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendRunStopMessage() {
  auto runData = RunData();
  // Flush producer queue to ensure the run stop is after all messages are
  // published
  m_publisher->flushSendQueue();
  runData.setStopTime(static_cast<uint64_t>(getTimeNowInNanoseconds() + 1));
  // + 1 as we want to include any messages which were sent in the current
  // nanosecond
  // (in the extremely unlikely event that it is possible to happen)

  auto buffer = runData.getRunStopBuffer();
  m_publisher->sendRunMessage(buffer);
  return buffer.size();
}

size_t NexusPublisher::createAndSendDetSpecMessage() {
  auto messageData = DetectorSpectrumMapData(m_detSpecMapFilename);
  auto messageBuffer = messageData.getBuffer();
  m_publisher->sendDetSpecMessage(messageBuffer);
  return messageBuffer.size();
}

/**
 * Display a progress bar
 *
 * @param progress - progress between 0 (starting) and 1 (complete)
 */
void NexusPublisher::reportProgress(const float progress) {
  if (!m_quietMode) {
    const int barWidth = 70;
    std::cout << "[";
    auto pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
      if (i < pos)
        std::cout << "=";
      else if (i == pos)
        std::cout << ">";
      else
        std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
  }
}
