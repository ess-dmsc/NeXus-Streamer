#pragma once

#include <memory>
#include <spdlog/spdlog.h>

#include "../../serialisation/include/DetectorSpectrumMapData.h"
#include "../../serialisation/include/EventData.h"
#include "../../serialisation/include/RunData.h"
#include "../../nexus_file_reader/include/FileReader.h"
#include "EventPublisher.h"
#include "OptionalArgs.h"

class NexusPublisher {
public:
  NexusPublisher(std::shared_ptr<EventPublisher> publisher,
                 std::shared_ptr<FileReader> fileReader,
                 const OptionalArgs &settings);
  std::vector<EventData> createMessageData(hsize_t frameNumber);
  size_t createAndSendRunMessage(int runNumber);
  size_t createAndSendDetSpecMessage();
  std::shared_ptr<RunData> createRunMessageData(int runNumber);
  std::shared_ptr<DetectorSpectrumMapData> createDetSpecMessageData();
  void streamData(int runNumber, bool slow,
                  std::pair<int32_t, int32_t> minMaxDetNums);

private:
  int64_t getTimeNowInNanoseconds();
  size_t createAndSendMessage(size_t frameNumber);
  void createAndSendSampleEnvMessages(size_t frameNumber);
  size_t createAndSendRunStopMessage();
  void reportProgress(float progress);

  std::shared_ptr<EventPublisher> m_publisher;
  std::shared_ptr<FileReader> m_fileReader;
  bool m_quietMode = false;
  std::string m_detSpecMapFilename;
  std::unordered_map<hsize_t, sEEventVector> m_sEEventMap;
  uint64_t m_messageID = 0;
  std::shared_ptr<spdlog::logger> m_logger = spdlog::get("LOG");
};
