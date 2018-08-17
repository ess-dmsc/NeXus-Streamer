#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include <h5cpp/hdf5.hpp>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

using sEEventVector = std::vector<std::shared_ptr<SampleEnvironmentEvent>>;

class NexusFileReader {
public:
  NexusFileReader(hdf5::file::File file, uint64_t runStartTime,
                  int32_t fakeEventsPerPulse,
                  const std::vector<int32_t> &detectorNumbers);

  hsize_t getFileSize();
  uint64_t getTotalEventCount();
  uint32_t getPeriodNumber();
  float getProtonCharge(hsize_t frameNumber);
  bool getEventDetIds(std::vector<uint32_t> &detIds, hsize_t frameNumber);
  bool getEventTofs(std::vector<uint32_t> &tofs, hsize_t frameNumber);
  size_t getNumberOfFrames() { return m_numberOfFrames; };
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber);
  uint64_t getFrameTime(hsize_t frameNumber);
  std::string getInstrumentName();
  std::unordered_map<hsize_t, sEEventVector> getSEEventMap();
  int32_t getNumberOfPeriods();
  void getEntryGroup(const hdf5::node::Group &rootGroup,
                     hdf5::node::Group &entryGroupOutput);
  void getEventGroup(const hdf5::node::Group &entryGroup,
                     hdf5::node::Group &eventGroupOutput);

private:
  uint64_t m_runStart;
  size_t findFrameNumberOfTime(float time);
  template <typename T>
  T getSingleValueFromDataset(const hdf5::node::Group &group,
                              const std::string &dataset, hsize_t offset);
  hsize_t getFrameStart(hsize_t frameNumber);
  size_t m_numberOfFrames;
  uint64_t m_frameStartOffset;

  hdf5::file::File m_file;
  hdf5::node::Group m_entryGroup;
  hdf5::node::Group m_eventGroup;
  hdf5::dataspace::Hyperslab m_slab{{0}, {1}};

  const int32_t m_fakeEventsPerPulse;

  /// Tools for generating events
  std::uniform_int_distribution<uint32_t> m_timeOfFlightDist;
  std::uniform_int_distribution<uint32_t> m_detectorIDDist;
  std::default_random_engine RandomEngine;

  std::vector<int32_t> m_detectorNumbers;
};
