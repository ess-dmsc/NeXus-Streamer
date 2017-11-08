#ifndef ISIS_NEXUS_STREAMER_NEXUSPUBLISHER_H
#define ISIS_NEXUS_STREAMER_NEXUSPUBLISHER_H

#include <memory>

#include "../../event_data/include/DetectorSpectrumMapData.h"
#include "../../event_data/include/EventData.h"
#include "../../event_data/include/RunData.h"
#include "../../nexus_file_reader/include/NexusFileReader.h"
#include "EventPublisher.h"

class NexusPublisher {
public:
  NexusPublisher(std::shared_ptr<EventPublisher> publisher,
                 const std::string &brokerAddress,
                 const std::string &instrumentName, const std::string &filename,
                 const std::string &detSpecMapFilename, const bool quietMode);
  std::vector<std::shared_ptr<EventData>>
  createMessageData(hsize_t frameNumber);
  size_t createAndSendRunMessage(std::string &rawbuf, int runNumber);
  size_t createAndSendDetSpecMessage(std::string &rawbuf);
  std::shared_ptr<RunData> createRunMessageData(int runNumber);
  std::shared_ptr<DetectorSpectrumMapData> createDetSpecMessageData();
  void streamData(int runNumber, bool slow);

private:
  size_t createAndSendMessage(std::string &rawbuf, size_t frameNumber);
  void createAndSendSampleEnvMessages(std::string &sampleEnvBuf,
                                      size_t frameNumber);
  size_t createAndSendRunStopMessage(std::string &rawbuf);
  void reportProgress(const float progress);

  std::shared_ptr<EventPublisher> m_publisher;
  std::shared_ptr<NexusFileReader> m_fileReader;
  bool m_quietMode = false;
  std::string m_detSpecMapFilename;
  std::unordered_map<hsize_t, sEEventVector> m_sEEventMap;
  uint64_t m_messageID = 0;
  uint64_t m_runStartTime;
};

#endif // ISIS_NEXUS_STREAMER_NEXUSPUBLISHER_H
