#include "../include/NexusFileReader.h"
#include "../../event_data/include/SampleEnvironmentEventDouble.h"
#include "../../event_data/include/SampleEnvironmentEventInt.h"
#include "../../event_data/include/SampleEnvironmentEventLong.h"
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace H5;

/**
 * Create a object to read the specified file
 *
 * @param filename - the full path of the NeXus file
 * @return - an object with which to read information from the file
 */
NexusFileReader::NexusFileReader(const std::string &filename,
                                 uint64_t runStartTime,
                                 const int32_t fakeEventsPerPulse,
                                 std::vector<int32_t> detectorNumbers)
    : m_file(new H5File(filename, H5F_ACC_RDONLY)), m_runStart(runStartTime),
      m_fakeEventsPerPulse(fakeEventsPerPulse),
      m_timeOfFlightDist(10000, 100000), m_detectorNumbers(detectorNumbers),
      m_detectorIDDist(0, static_cast<uint32_t>(detectorNumbers.size() - 1)) {
  DataSet dataset = m_file->openDataSet("/raw_data_1/good_frames");
  size_t numOfFrames;
  dataset.read(&numOfFrames, PredType::NATIVE_UINT64);
  m_numberOfFrames = numOfFrames;
  // Use pulse times relative to start time rather than using the `offset`
  // attribute from the NeXus file, this makes the timestamps look as if this
  // data is coming from a live instrument
  m_frameStartOffset = m_runStart;
}

size_t NexusFileReader::findFrameNumberOfTime(float time) {
  auto frameNumber = static_cast<size_t>(std::floor(time / 0.1));
  return frameNumber;
}

std::unordered_map<hsize_t, sEEventVector> NexusFileReader::getSEEventMap() {
  std::unordered_map<hsize_t, sEEventVector> sEEventMap;

  auto groupNames = getNamesInGroup("/raw_data_1/selog");
  for (auto const &name : groupNames) {
    if ((name != "SECI_OUT_OF_RANGE_BLOCK") && (name != "gas_control")) {
      std::vector<float> floatValues;
      std::vector<int32_t> intValues;
      std::vector<int64_t> longValues;
      std::string valueDatasetName =
          "/raw_data_1/selog/" + name + "/value_log/value";
      auto times =
          get1DDataset<float>(PredType::NATIVE_FLOAT,
                              "/raw_data_1/selog/" + name + "/value_log/time");
      auto valueType = getDatasetType(valueDatasetName);
      if (valueType == PredType::NATIVE_FLOAT) {
        floatValues = get1DDataset<float>(valueType, valueDatasetName);
      } else if (valueType == PredType::NATIVE_INT32) {
        intValues = get1DDataset<int32_t>(valueType, valueDatasetName);
      } else if (valueType == PredType::NATIVE_INT64) {
        longValues = get1DDataset<int64_t>(valueType, valueDatasetName);
      } else {
        std::cout << "Unsupported datatype in dataset " << name << std::endl;
        continue;
      }

      for (size_t i = 0; i < times.size(); i++) {
        // Ignore entries for events which do not occur during the run
        if (times[i] > 0) {
          // The number of the frame the event happened in
          auto frameNumber = findFrameNumberOfTime(times[i]);
          if (frameNumber > m_numberOfFrames) {
            continue;
          }
          if (sEEventMap.count(frameNumber) == 0)
            sEEventMap[frameNumber] = sEEventVector();
          if (valueType == PredType::NATIVE_FLOAT)
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventDouble>(
                    name, times[i], floatValues[i], m_runStart));
          else if (valueType == PredType::NATIVE_INT32)
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventInt>(
                    name, times[i], intValues[i], m_runStart));
          else if (valueType == PredType::NATIVE_INT64)
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventLong>(
                    name, times[i], longValues[i], m_runStart));
        }
      }
    }
  }

  return sEEventMap;
}

DataType NexusFileReader::getDatasetType(const std::string &datasetName) {
  auto dataset = m_file->openDataSet(datasetName);
  return dataset.getDataType();
}

template <typename valueType>
std::vector<valueType>
NexusFileReader::get1DDataset(DataType dataType,
                              const std::string &datasetName) {
  auto dataset = m_file->openDataSet(datasetName);
  std::vector<valueType> values;

  auto dataspace = dataset.getSpace();

  // resize vector to the correct size to put the new data in
  values.resize(static_cast<size_t>(dataspace.getSelectNpoints()));

  dataset.read(values.data(), dataType, dataspace);
  return values;
}

std::vector<std::string>
NexusFileReader::get1DStringDataset(const std::string &datasetName) {
  std::string value;
  auto dataset = m_file->openDataSet(datasetName);
  auto dataspace = dataset.getSpace();

  dataset.read(value, dataset.getDataType(), dataspace);

  std::vector<std::string> values;
  values.push_back(value);

  return values;
}

/**
 * Get the size of the NeXus file in bytes
 *
 * @return - the size of the file in bytes
 */
hsize_t NexusFileReader::getFileSize() { return m_file->getFileSize(); }

/**
 * Get the total number of events in the file
 *
 * @return - total number of events
 */
uint64_t NexusFileReader::getTotalEventCount() {
  if (m_fakeEventsPerPulse > 0) {
    return getNumberOfFrames() * m_fakeEventsPerPulse;
  }

  DataSet dataset =
      m_file->openDataSet("/raw_data_1/detector_1_events/total_counts");
  uint64_t totalCount;
  dataset.read(&totalCount, PredType::NATIVE_UINT64);

  return totalCount;
}

/**
 * Get the DAE period number
 *
 * @return - the DAE period number
 */
uint32_t NexusFileReader::getPeriodNumber() {
  DataSet dataset = m_file->openDataSet("/raw_data_1/periods/number");
  int32_t periodNumber;
  dataset.read(&periodNumber, PredType::NATIVE_INT32);
  // -1 as period number starts at 1 in NeXus files but 0 everywhere else
  periodNumber -= 1;
  if (periodNumber < 0)
    throw std::runtime_error(
        "Period number in NeXus file is expected to be > 0");

  return static_cast<uint32_t>(periodNumber);
}

/**
 * Get the number of DAE periods
 *
 * @return - the number of periods
 */
int32_t NexusFileReader::getNumberOfPeriods() {
  auto periodNumbers = get1DDataset<int32_t>(PredType::NATIVE_INT32,
                                             "/raw_data_1/periods/number");
  return static_cast<int32_t>(periodNumbers.size());
}

/**
 * Get the names of objects in a specified group
 *
 * @param name of the group
 * @return names of objects in the specified group
 */
std::vector<std::string>
NexusFileReader::getNamesInGroup(const std::string &groupName) {
  Group group = m_file->openGroup(groupName);
  std::vector<std::string> names;
  for (hsize_t i = 0; i < group.getNumObjs(); i++) {
    names.push_back(group.getObjnameByIdx(i));
  }
  return names;
}

/**
 * Get instrument name
 *
 * @return - instrument name
 */
std::string NexusFileReader::getInstrumentName() {
  DataSet dataset = m_file->openDataSet("/raw_data_1/name");
  std::string instrumentName;
  dataset.read(instrumentName, dataset.getDataType(), dataset.getSpace());
  return instrumentName;
}

/**
 * Get the proton charge
 *
 * @return - the proton charge
 */
float NexusFileReader::getProtonCharge(hsize_t frameNumber) {
  std::string datasetName = "/raw_data_1/framelog/proton_charge/value";

  auto protonCharge = getSingleValueFromDataset<float>(
      datasetName, PredType::NATIVE_FLOAT, frameNumber);

  return protonCharge;
}

/**
 * Gets the absolute time of the start of the specified frame
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - absolute time of frame start in nanoseconds since 1 Jan 1970
 */
uint64_t NexusFileReader::getFrameTime(hsize_t frameNumber) {
  std::string datasetName = "/raw_data_1/detector_1_events/event_time_zero";

  auto frameTime = getSingleValueFromDataset<double>(
      datasetName, PredType::NATIVE_DOUBLE, frameNumber);
  auto frameTimeFromOffsetNanoseconds =
      static_cast<uint64_t>(floor((frameTime * 1e9) + 0.5));
  return m_frameStartOffset + frameTimeFromOffsetNanoseconds;
}

template <typename T>
T NexusFileReader::getSingleValueFromDataset(const std::string &datasetName,
                                             H5::PredType datatype,
                                             hsize_t offset) {
  auto dataset = m_file->openDataSet(datasetName);
  T value;

  hsize_t count = 1;
  hsize_t stride = 1;
  hsize_t block = 1;

  auto dataspace = dataset.getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, &count, &offset, &stride, &block);

  hsize_t dimsm = 1;
  DataSpace memspace(1, &dimsm);

  dataset.read(&value, datatype, memspace, dataspace);

  return value;
}

/**
 * Gets the event index of the start of the specified frame
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - event index corresponding to the start of the specified frame
 */
hsize_t NexusFileReader::getFrameStart(hsize_t frameNumber) {
  std::string datasetName = "/raw_data_1/detector_1_events/event_index";

  auto frameStart = getSingleValueFromDataset<hsize_t>(
      datasetName, PredType::NATIVE_UINT64, frameNumber);

  return frameStart;
}

/**
 * Get the number of events which are in the specified frame
 *
 * @param frameNumber - the number of the frame in which to count the number of
 * events
 * @return - the number of events in the specified frame
 */
hsize_t NexusFileReader::getNumberOfEventsInFrame(hsize_t frameNumber) {
  if (m_fakeEventsPerPulse > 0) {
    return static_cast<hsize_t>(m_fakeEventsPerPulse);
  }
  // if this is the last frame then we cannot get number of events by looking at
  // event index of next frame
  // instead use the total_counts field
  if (frameNumber == (m_numberOfFrames - 1)) {
    return getTotalEventCount() - getFrameStart(frameNumber);
  }
  return getFrameStart(frameNumber + 1) - getFrameStart(frameNumber);
}

/**
 * Get the list of detector IDs corresponding to events in the specifed frame
 *
 * @param detIds - vector in which to store the detector IDs
 * @param frameNumber - the number of the frame in which to get the detector IDs
 * @return - false if the specified frame number is not the data range, true
 * otherwise
 */
bool NexusFileReader::getEventDetIds(std::vector<uint32_t> &detIds,
                                     hsize_t frameNumber) {
  if (frameNumber >= m_numberOfFrames)
    return false;

  if (m_fakeEventsPerPulse > 0) {
    detIds.reserve(static_cast<size_t>(m_fakeEventsPerPulse));
    for (size_t i = 0; i < m_fakeEventsPerPulse; i++) {
      detIds.push_back(static_cast<uint32_t>(
          m_detectorNumbers[m_detectorIDDist(RandomEngine)]));
    }
    return true;
  }

  auto dataset = m_file->openDataSet("/raw_data_1/detector_1_events/event_id");

  auto numberOfEventsInFrame = getNumberOfEventsInFrame(frameNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber);
  hsize_t stride = 1;
  hsize_t block = 1;

  auto dataspace = dataset.getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, &count, &offset, &stride, &block);

  // resize detIds to the correct size to put the new data in
  detIds.resize(numberOfEventsInFrame);

  hsize_t dimsm = numberOfEventsInFrame;
  DataSpace memspace(1, &dimsm);

  dataset.read(detIds.data(), PredType::NATIVE_UINT32, memspace, dataspace);

  return true;
}

/**
 * Get the list of flight times corresponding to events in the specifed frame
 *
 * @param tofs - vector in which to store the time-of-flight
 * @param frameNumber - the number of the frame in which to get the
 * time-of-flights
 * @return - false if the specified frame number is not the data range, true
 * otherwise
 */
bool NexusFileReader::getEventTofs(std::vector<uint32_t> &tofs,
                                   hsize_t frameNumber) {
  if (frameNumber >= m_numberOfFrames)
    return false;

  if (m_fakeEventsPerPulse > 0) {
    tofs.reserve(static_cast<size_t>(m_fakeEventsPerPulse));
    for (size_t i = 0; i < m_fakeEventsPerPulse; i++) {
      tofs.push_back(static_cast<uint32_t>(m_timeOfFlightDist(RandomEngine)));
    }
    return true;
  }

  auto dataset =
      m_file->openDataSet("/raw_data_1/detector_1_events/event_time_offset");

  auto numberOfEventsInFrame = getNumberOfEventsInFrame(frameNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber);
  hsize_t stride = 1;
  hsize_t block = 1;

  std::vector<float> tof_floats;

  auto dataspace = dataset.getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, &count, &offset, &stride, &block);

  tofs.resize(numberOfEventsInFrame);
  tof_floats.resize(numberOfEventsInFrame);

  hsize_t dimsm = numberOfEventsInFrame;
  DataSpace memspace(1, &dimsm);

  dataset.read(tof_floats.data(), PredType::NATIVE_FLOAT, memspace, dataspace);
  // transform float in microseconds to uint32 in nanoseconds
  std::transform(tof_floats.begin(), tof_floats.end(), tofs.begin(),
                 [](float tof) {
                   return static_cast<uint32_t>(floor((tof * 1000) + 0.5));
                 });

  return true;
}
