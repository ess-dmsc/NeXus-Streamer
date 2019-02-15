#pragma once

#include <memory>
#include <spdlog/spdlog.h>

#include "../../nexus_file_reader/include/FileReader.h"
#include "EventPublisher.h"
#include "OptionalArgs.h"

class EventData;
class RunData;

class NexusPublisher {
public:
  NexusPublisher(std::shared_ptr<EventPublisher> publisher,
                 std::shared_ptr<FileReader> fileReader,
                 const OptionalArgs &settings);
  size_t createAndSendRunMessage(int runNumber);
  size_t createAndSendDetSpecMessage();
  std::vector<EventData> createMessageData(hsize_t frameNumber);
  void streamData(int runNumber, bool slow,
                  std::pair<int32_t, int32_t> minMaxDetNums);

private:
  RunData createRunMessageData(int runNumber);
  int64_t getTimeNowInNanoseconds();
  size_t createAndSendMessage(size_t frameNumber);
  void createAndSendSampleEnvMessages(size_t frameNumber);
  size_t createAndSendRunStopMessage();
  size_t createAndSendHistogramMessage(uint64_t timestamp);
  void reportProgress(float progress);

  std::shared_ptr<EventPublisher> m_publisher;
  std::shared_ptr<FileReader> m_fileReader;
  bool m_quietMode = false;
  std::string m_detSpecMapFilename;
  std::unordered_map<hsize_t, sEEventVector> m_sEEventMap;
  uint64_t m_messageID = 0;
  std::shared_ptr<spdlog::logger> m_logger = spdlog::get("LOG");
};
