#include <chrono>
#include <iostream>
#include <thread>

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
                               const std::string &streamName,
                               const std::string &runTopicName,
                               const std::string &filename,
                               const bool quietMode)
    : m_publisher(publisher),
      m_fileReader(std::make_shared<NexusFileReader>(filename)),
      m_quietMode(quietMode) {
  publisher->setUp(brokerAddress, streamName, runTopicName);
}

/**
 * For a given frame number, reads the data from file and stores them in
 * messagesPerFrame EventData objects
 *
 * @param frameNumber - the number of the frame for which to construct a message
 * @return - an object containing the data from the specified frame
 */
std::vector<std::shared_ptr<EventData>>
NexusPublisher::createMessageData(hsize_t frameNumber,
                                  const int messagesPerFrame) {
  std::vector<std::shared_ptr<EventData>> eventDataVector;

  std::vector<int32_t> detIds;
  m_fileReader->getEventDetIds(detIds, frameNumber);
  std::vector<float> tofs;
  m_fileReader->getEventTofs(tofs, frameNumber);

  auto numberOfFrames = m_fileReader->getNumberOfFrames();
  // this assumes constant current during frame, which I think is all I can do
  // with data in the nexus file
  auto protonCharge =
      m_fileReader->getProtonCharge(frameNumber) / messagesPerFrame;
  auto period = m_fileReader->getPeriodNumber();
  auto frameTime = m_fileReader->getFrameTime(frameNumber);

  uint32_t eventsPerMessage =
      static_cast<uint32_t>(ceil(static_cast<double>(detIds.size()) /
                                 static_cast<double>(messagesPerFrame)));
  for (int messageNumber = 0; messageNumber < messagesPerFrame;
       messageNumber++) {
    auto eventData = std::make_shared<EventData>();
    eventData->setProtonCharge(protonCharge);
    eventData->setPeriod(period);
    eventData->setFrameTime(frameTime);

    auto upToDetId = detIds.begin() + ((messageNumber + 1) * eventsPerMessage);
    auto upToTof = tofs.begin() + ((messageNumber + 1) * eventsPerMessage);

    // The last message of the frame will contain any remaining events
    if (messageNumber == (messagesPerFrame - 1)) {
      upToDetId = detIds.end();
      upToTof = tofs.end();
      eventData->setEndOfFrame(true);
      if (frameNumber == (numberOfFrames - 1)) {
        eventData->setEndOfRun(true);
      }
    }

    std::vector<int32_t> detIdsCurrentMessage(
        detIds.begin() + (messageNumber * eventsPerMessage), upToDetId);
    std::vector<float> tofsCurrentMessage(
        tofs.begin() + (messageNumber * eventsPerMessage), upToTof);

    eventData->setDetId(detIdsCurrentMessage);
    eventData->setTof(tofsCurrentMessage);
    eventData->setFrameNumber(static_cast<uint32_t>(frameNumber));
    eventData->setTotalCounts(m_fileReader->getTotalEventCount());

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
  runData->setInstrumentName(m_fileReader->getInstrumentName());
  runData->setRunNumber(runNumber);
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  runData->setStartTime(static_cast<uint64_t>(now_c));
  return runData;
}

/**
 * Start streaming all the data from the file
 */
void NexusPublisher::streamData(const int messagesPerFrame, int runNumber,
                                bool slow) {
  std::string rawbuf;
  // frame numbers run from 0 to numberOfFrames-1
  reportProgress(0.0);
  int64_t totalBytesSent = 0;
  const auto numberOfFrames = m_fileReader->getNumberOfFrames();

  totalBytesSent += createAndSendRunMessage(rawbuf, runNumber);

  for (size_t frameNumber = 0; frameNumber < numberOfFrames; frameNumber++) {
    totalBytesSent +=
        createAndSendMessage(rawbuf, frameNumber, messagesPerFrame);
    reportProgress(static_cast<float>(frameNumber) /
                   static_cast<float>(numberOfFrames));

    // Publish messages at roughly realistic messsage rate (~10 frames per
    // second)
    if (slow) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  reportProgress(1.0);
  std::cout << std::endl
            << "Frames sent: " << m_fileReader->getNumberOfFrames() << std::endl
            << "Bytes sent: " << totalBytesSent << std::endl
            << "Average message size: "
            << totalBytesSent / (messagesPerFrame * numberOfFrames * 1000)
            << " kB" << std::endl;
}

/**
 * Using Google Flatbuffers, create a message for the specifed frame and send it
 *
 * @param rawbuf - a buffer for the message
 * @param frameNumber - the number of the frame for which data will be sent
 * @return - size of the buffer
 */
int64_t NexusPublisher::createAndSendMessage(std::string &rawbuf,
                                             size_t frameNumber,
                                             const int messagesPerFrame) {
  auto messageData = createMessageData(frameNumber, messagesPerFrame);
  int64_t dataSize = 0;
  for (const auto &message : messageData) {
    auto buffer_uptr = message->getBufferPointer(rawbuf);
    m_publisher->sendEventMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                                  message->getBufferSize());
    dataSize += rawbuf.size();
  }
  return dataSize;
}

/**
 * Create a message containing run metadata and send it
 *
 * @param rawbuf - a buffer for the message
 * @param runNumber - integer to identify the run
 * @return - size of the buffer
 */
int64_t NexusPublisher::createAndSendRunMessage(std::string &rawbuf,
                                                int runNumber) {
  auto messageData = createRunMessageData(runNumber);
  messageData->setStreamOffset(m_publisher->getCurrentOffset());
  auto buffer_uptr = messageData->getEventBufferPointer(rawbuf);
  // publish to both topics
  m_publisher->sendEventMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                                messageData->getBufferSize());
  buffer_uptr = messageData->getRunBufferPointer(rawbuf);
  m_publisher->sendRunMessage(reinterpret_cast<char *>(buffer_uptr.get()),
                              messageData->getBufferSize());
  std::cout << "Publishing new run:" << std::endl;
  std::cout << messageData->runInfo() << std::endl;
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
