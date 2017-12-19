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
                               const std::string &brokerAddress,
                               const std::string &instrumentName,
                               const std::string &filename,
                               const std::string &detSpecMapFilename,
                               const bool quietMode)
    : m_publisher(publisher), m_quietMode(quietMode),
      m_detSpecMapFilename(detSpecMapFilename) {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  m_runStartTime = static_cast<uint64_t>(now_c) * 1000000000L;
  m_fileReader = std::make_shared<NexusFileReader>(filename, m_runStartTime);
  publisher->setUp(brokerAddress, instrumentName);
  m_sEEventMap = m_fileReader->getSEEventMap();
}

/**
 * For a given frame number, reads the data from file and stores them in
 * messagesPerFrame EventData objects
 *
 * @param frameNumber - the number of the frame for which to construct a message
 * @return - an object containing the data from the specified frame
 */
std::vector<std::shared_ptr<EventData>>
NexusPublisher::createMessageData(hsize_t frameNumber) {
  std::vector<std::shared_ptr<EventData>> eventDataVector;

  std::vector<uint32_t> detIds;
  m_fileReader->getEventDetIds(detIds, frameNumber);
  std::vector<uint32_t> tofs;
  m_fileReader->getEventTofs(tofs, frameNumber);

  auto protonCharge = m_fileReader->getProtonCharge(frameNumber);
  auto period = m_fileReader->getPeriodNumber();
  auto frameTime = m_fileReader->getFrameTime(frameNumber);

  auto eventData = std::make_shared<EventData>();
  eventData->setProtonCharge(protonCharge);
  eventData->setPeriod(period);
  eventData->setFrameTime(frameTime);
  eventData->setDetId(detIds);
  eventData->setTof(tofs);
  eventData->setTotalCounts(m_fileReader->getTotalEventCount());

  eventDataVector.push_back(eventData);

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
void NexusPublisher::streamData(int runNumber, bool slow) {
  std::string rawbuf;
  std::string sampleEnvBuf;
  // frame numbers run from 0 to numberOfFrames-1
  int64_t totalBytesSent = 0;
  const auto numberOfFrames = m_fileReader->getNumberOfFrames();

  totalBytesSent += createAndSendRunMessage(rawbuf, runNumber);
  totalBytesSent += createAndSendDetSpecMessage(rawbuf);

  for (size_t frameNumber = 0; frameNumber < numberOfFrames; frameNumber++) {
    totalBytesSent += createAndSendMessage(rawbuf, frameNumber);
    createAndSendSampleEnvMessages(sampleEnvBuf, frameNumber);
    reportProgress(static_cast<float>(frameNumber) /
                   static_cast<float>(numberOfFrames));

    // Publish messages at roughly realistic message rate (~10 frames per
    // second)
    if (slow) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  totalBytesSent += createAndSendRunStopMessage(rawbuf);
  reportProgress(1.0);
  std::cout << std::endl
            << "Frames sent: " << m_fileReader->getNumberOfFrames() << std::endl
            << "Bytes sent: " << totalBytesSent << std::endl;
}

/**
 * Using Google Flatbuffers, create a message for the specifed frame and send it
 *
 * @param rawbuf - a buffer for the message
 * @param frameNumber - the number of the frame for which data will be sent
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendMessage(std::string &rawbuf,
                                            size_t frameNumber) {
  auto messageData = createMessageData(frameNumber);
  std::vector<int> indexes;
  indexes.reserve(messageData.size());
  for (int i = 0; i < messageData.size(); ++i)
    indexes.push_back(i);
  size_t dataSize = 0;
  for (const auto &index : indexes) {
    auto buffer_uptr =
        messageData[index]->getBufferPointer(rawbuf, m_messageID + index);
    m_publisher->sendEventMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                                  messageData[index]->getBufferSize());
    dataSize += rawbuf.size();
  }
  m_messageID += indexes.size();
  return dataSize;
}

/**
 * Create a flatbuffer payload for sample environment log messages
 *
 * @param sampleEnvBuf - a buffer for the message
 * @param frameNumber - the number of the frame for which data will be sent
 */
void NexusPublisher::createAndSendSampleEnvMessages(std::string &sampleEnvBuf,
                                                    size_t frameNumber) {
  for (const auto &sEEvent : m_sEEventMap[frameNumber]) {
    auto buffer_uptr = sEEvent->getBufferPointer(sampleEnvBuf);
    m_publisher->sendSampleEnvMessage(
        reinterpret_cast<char *>(buffer_uptr.get()), sEEvent->getBufferSize());
  }
}

/**
 * Create a message containing run metadata and send it
 *
 * @param rawbuf - a buffer for the message
 * @param runNumber - integer to identify the run
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendRunMessage(std::string &rawbuf,
                                               int runNumber) {
  auto messageData = createRunMessageData(runNumber);
  auto buffer_uptr = messageData->getRunStartBufferPointer(rawbuf);
  m_publisher->sendRunMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                              messageData->getBufferSize());
  std::cout << std::endl
            << "Publishing new run:" << std::endl;
  std::cout << messageData->runInfo() << std::endl;
  return rawbuf.size();
}

/**
 * Signal that the end of the run has been reached
 *
 * @param rawbuf - a buffer for the message
 * @return - size of the buffer
 */
size_t NexusPublisher::createAndSendRunStopMessage(std::string &rawbuf) {
  auto runData = std::make_shared<RunData>();
  // Flush producer queue to ensure the run stop is after all messages are
  // published
  m_publisher->flushSendQueue();
  runData->setStopTime(static_cast<uint64_t>(getTimeNowInNanoseconds() + 1));
  // + 1 as we want to include any messages which were sent in the current
  // nanosecond
  // (in the extremely unlikely event that it is possible to happen)

  auto buffer_uptr = runData->getRunStopBufferPointer(rawbuf);
  m_publisher->sendRunMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                              runData->getBufferSize());
  return rawbuf.size();
}

int64_t NexusPublisher::getTimeNowInNanoseconds() {
  auto now = std::chrono::system_clock::now();
  auto now_epoch = now.time_since_epoch();
  auto now_epoch_nanoseconds =
    std::chrono::duration_cast<std::chrono::nanoseconds>(now_epoch).count();
  return now_epoch_nanoseconds;
}

size_t NexusPublisher::createAndSendDetSpecMessage(std::string &rawbuf) {
  auto messageData = createDetSpecMessageData();
  auto buffer_uptr = messageData->getBufferPointer(rawbuf);
  m_publisher->sendDetSpecMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                                  messageData->getBufferSize());
  return rawbuf.size();
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
