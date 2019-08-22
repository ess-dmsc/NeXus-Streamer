#pragma once

#include <memory>
#include <spdlog/spdlog.h>

#include "../../core/include/OptionalArgs.h"
#include "../../nexus_file_reader/include/FileReader.h"
#include "Publisher.h"

class EventData;
class RunData;
class Timer;

class NexusPublisher {
public:
  NexusPublisher(std::shared_ptr<Publisher> publisher,
                 std::shared_ptr<FileReader> fileReader,
                 const OptionalArgs &settings);
  std::vector<EventData> createMessageData(hsize_t frameNumber);
  void streamData(int runNumber, const OptionalArgs &settings,
                  const std::string &jsonDescription);

private:
  std::unique_ptr<Timer>
  publishHistogramBatch(const std::vector<HistogramFrame> &histograms,
                        uint32_t histogramUpdatePeriodMs,
                        int32_t numberOfTimerIterations);
  std::unique_ptr<Timer> streamHistogramData(const OptionalArgs &settings);
  size_t createAndSendRunMessage(int runNumber,
                                 const std::string &jsonDescription);
  size_t createAndSendDetSpecMessage();
  RunData createRunMessageData(int runNumber,
                               const std::string &jsonDescription);
  size_t createAndSendMessage(size_t frameNumber);
  void createAndSendSampleEnvMessages(size_t frameNumber);
  size_t createAndSendRunStopMessage(int runNumber);
  void reportProgress(float progress);

  const OptionalArgs m_settings;
  std::shared_ptr<Publisher> m_publisher;
  std::shared_ptr<FileReader> m_fileReader;
  std::string m_detSpecMapFilename;
  std::unordered_map<hsize_t, sEEventVector> m_sEEventMap;
  uint64_t m_messageID = 0;
  std::shared_ptr<spdlog::logger> m_logger = spdlog::get("LOG");
};
