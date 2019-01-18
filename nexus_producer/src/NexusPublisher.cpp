#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "../../event_data/include/DetectorSpectrumMapData.h"
#include "NexusPublisher.h"

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
NexusPublisher::NexusPublisher(std::shared_ptr<EventPublisher> publisher,
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
std::shared_ptr<RunData> NexusPublisher::createRunMessageData(int runNumber) {
  auto runData = std::make_shared<RunData>();
  runData->setNumberOfPeriods(m_fileReader->getNumberOfPeriods());
  runData->setInstrumentName(m_fileReader->getInstrumentName());
  runData->setRunNumber(runNumber);
  runData->setStartTime(static_cast<uint64_t>(getTimeNowInNanoseconds()));
  return runData;
}

/**
 * Create detector-spectrum map message data from file
 *
 * @return detector-spectrum map message data
 */
std::shared_ptr<DetectorSpectrumMapData>
NexusPublisher::createDetSpecMessageData() {
  auto detSpecMap =
      std::make_shared<DetectorSpectrumMapData>(m_detSpecMapFilename);
  return detSpecMap;
}

/**
 * Start streaming all the data from the file
 */
void NexusPublisher::streamData(int runNumber, bool slow,
                                std::pair<int32_t, int32_t> minMaxDetNums) {
  // frame numbers run from 0 to numberOfFrames-1
  int64_t totalBytesSent = 0;
  const auto numberOfFrames = m_fileReader->getNumberOfFrames();

  totalBytesSent += createAndSendRunMessage(runNumber);
  if (minMaxDetNums.first == 0 && minMaxDetNums.second == 0) {
    totalBytesSent += createAndSendDetSpecMessage();
  }

  uint64_t lastFrameTime = 0;
  for (size_t frameNumber = 0; frameNumber < numberOfFrames; frameNumber++) {
    // Publish messages at approx real message rate
    if (slow) {
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
  totalBytesSent += createAndSendRunStopMessage();
  reportProgress(1.0);
  std::cout << std::endl;

  m_logger->info("Frames sent: {}, Bytes sent: {}",
                 m_fileReader->getNumberOfFrames(), totalBytesSent);
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
  auto buffer = messageData->getRunStartBuffer();
  m_publisher->sendRunMessage(buffer);
  m_logger->info("Publishing new run: {}", messageData->runInfo());
  return buffer.size();
}

/**
 * Signal that the end of the run has been reached
 *
 * @param rawbuf - a buffer for the message
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendRunStopMessage() {
  auto runData = std::make_shared<RunData>();
  // Flush producer queue to ensure the run stop is after all messages are
  // published
  m_publisher->flushSendQueue();
  runData->setStopTime(static_cast<uint64_t>(getTimeNowInNanoseconds() + 1));
  // + 1 as we want to include any messages which were sent in the current
  // nanosecond
  // (in the extremely unlikely event that it is possible to happen)

  auto buffer = runData->getRunStopBuffer();
  m_publisher->sendRunMessage(buffer);
  return buffer.size();
}

int64_t NexusPublisher::getTimeNowInNanoseconds() {
  auto now = std::chrono::system_clock::now();
  auto now_epoch = now.time_since_epoch();
  auto now_epoch_nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(now_epoch).count();
  return now_epoch_nanoseconds;
}

size_t NexusPublisher::createAndSendDetSpecMessage() {
  auto messageData = createDetSpecMessageData();
  auto messageBuffer = messageData->getBuffer();
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
