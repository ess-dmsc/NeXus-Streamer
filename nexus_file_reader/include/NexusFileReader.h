#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include <H5Cpp.h>
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>

// a typedef for our managed H5File pointer
using H5FilePtr = std::unique_ptr<H5::H5File>;

using sEEventVector = std::vector<std::shared_ptr<SampleEnvironmentEvent>>;

class NexusFileReader {
public:
  NexusFileReader(const std::string &filename, uint64_t runStartTime,
                  int32_t fakeEventsPerPulse,
                  std::vector<int32_t> detectorNumbers);

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
  std::vector<std::string> getNamesInGroup(const std::string &groupName);
  std::unordered_map<hsize_t, sEEventVector> getSEEventMap();
  H5::DataType getDatasetType(const std::string &datasetName);
  template <typename valueType>
  std::vector<valueType> get1DDataset(H5::DataType dataType,
                                      const std::string &datasetName);
  std::vector<std::string> get1DStringDataset(const std::string &datasetName);
  int32_t getNumberOfPeriods();
  uint64_t getFrameStartOffset();

private:
  uint64_t m_runStart;
  size_t findFrameNumberOfTime(float time);
  template <typename T>
  T getSingleValueFromDataset(const std::string &dataset, H5::PredType datatype,
                              hsize_t offset);
  hsize_t getFrameStart(hsize_t frameNumber);
  H5FilePtr m_file = nullptr;
  size_t m_numberOfFrames;
  uint64_t convertStringToUnixTime(const std::string &timeString);
  uint64_t m_frameStartOffset;
  const int32_t m_fakeEventsPerPulse;

  /// Tools for generating events
  std::uniform_int_distribution<uint32_t> m_timeOfFlightDist;
  std::uniform_int_distribution<uint32_t> m_detectorIDDist;
  std::default_random_engine RandomEngine;

  std::vector<int32_t> m_detectorNumbers;
};
