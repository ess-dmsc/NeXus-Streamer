#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include <H5Ipublic.h>
#include <h5cpp/hdf5.hpp>
#include <hdf5.h>
#include <memory>
#include <unordered_map>
#include <vector>

using sEEventVector = std::vector<std::shared_ptr<SampleEnvironmentEvent>>;

class NexusFileReader {
public:
  NexusFileReader(const std::string &filename, uint64_t runStartTime);

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
  H5::DataType getDatasetType(const std::string &datasetName);
  template <typename valueType>
  std::vector<valueType> get1DDataset(H5::DataType dataType,
                                      const std::string &datasetName);
  std::vector<std::string> get1DStringDataset(const std::string &datasetName);
  int32_t getNumberOfPeriods();
  uint64_t getFrameStartOffset();
  bool getEntryGroup(const hdf5::node::Group &rootGroup,
                     hdf5::node::Group &entryGroupOutput);

private:
  uint64_t m_runStart;
  size_t findFrameNumberOfTime(float time);
  template <typename T>
  T getSingleValueFromDataset(const std::string &dataset, H5::PredType datatype,
                              hsize_t offset);
  hsize_t getFrameStart(hsize_t frameNumber);
  size_t m_numberOfFrames;
  uint64_t convertStringToUnixTime(const std::string &timeString);
  uint64_t m_frameStartOffset;

  hdf5::file::File m_file;
  hdf5::node::Group m_entryGroup;
};
