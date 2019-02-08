#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include "FileReader.h"
#include <h5cpp/hdf5.hpp>
#include <memory>
#include <random>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

class NexusFileReader : public FileReader {
public:
  NexusFileReader(hdf5::file::File file, uint64_t runStartTime,
                  int32_t fakeEventsPerPulse,
                  const std::vector<int32_t> &detectorNumbers);

  hsize_t getFileSize() override;
  uint64_t getTotalEventCount() override;
  uint32_t getPeriodNumber() override;
  float getProtonCharge(hsize_t frameNumber) override;
  std::vector<EventDataFrame> getEventData(hsize_t frameNumber) override;
  size_t getNumberOfFrames() override { return m_numberOfFrames; };
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber,
                                   size_t eventGroupNumber) override;
  uint64_t getFrameTime(hsize_t frameNumber) override;
  std::string getInstrumentName() override;
  std::unordered_map<hsize_t, sEEventVector> getSEEventMap() override;
  int32_t getNumberOfPeriods() override;
  uint64_t getRelativeFrameTimeMilliseconds(hsize_t frameNumber) override;
  bool isISISFile() override;
  uint64_t getTotalEventsInGroup(size_t eventGroupNumber) override;

private:
  std::vector<uint32_t> getEventDetIds(hsize_t frameNumber,
                                       size_t eventGroupNumber);
  std::vector<uint32_t> getEventTofs(hsize_t frameNumber,
                                     size_t eventGroupNumber);
  void getEntryGroup(const hdf5::node::Group &rootGroup,
                     hdf5::node::Group &entryGroupOutput);
  void getEventGroups(const hdf5::node::Group &entryGroup,
                      std::vector<hdf5::node::Group> &eventGroupsOutput);
  size_t findFrameNumberOfTime(float time);
  std::vector<hdf5::node::Group> findNXLogs();
  template <typename T>
  T getSingleValueFromDataset(const hdf5::node::Group &group,
                              const std::string &dataset, hsize_t offset);
  hsize_t getFrameStart(hsize_t frameNumber, size_t eventGroupNumber);
  void
  checkEventGroupHasRequiredDatasets(const hdf5::node::Group &eventGroup) const;
  bool testIfIsISISFile();

  size_t m_numberOfFrames;
  uint64_t m_frameStartOffset;

  hdf5::file::File m_file;
  hdf5::node::Group m_entryGroup;
  std::vector<hdf5::node::Group> m_eventGroups;
  hdf5::dataspace::Hyperslab m_slab{{0}, {1}};

  uint64_t m_runStart;
  const int32_t m_fakeEventsPerPulse;

  std::vector<int32_t> m_detectorNumbers;

  /// Tools for generating events
  std::uniform_int_distribution<uint32_t> m_timeOfFlightDist;
  std::uniform_int_distribution<uint32_t> m_detectorIDDist;
  std::default_random_engine RandomEngine;

  bool m_isisFile;

  std::shared_ptr<spdlog::logger> m_logger = spdlog::get("LOG");
};
