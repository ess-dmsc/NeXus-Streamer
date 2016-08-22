#include "../include/NexusFileReader.h"
#include "../../event_data/include/SampleEnvironmentEventDouble.h"
#include "../../event_data/include/SampleEnvironmentEventInt.h"
#include "../../event_data/include/SampleEnvironmentEventLong.h"
#include "../../event_data/include/SampleEnvironmentEventString.h"
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
NexusFileReader::NexusFileReader(const std::string &filename)
    : m_file(new H5File(filename, H5F_ACC_RDONLY)) {
  DataSet dataset = m_file->openDataSet("/raw_data_1/good_frames");
  size_t numOfFrames;
  dataset.read(&numOfFrames, PredType::NATIVE_UINT64);
  m_numberOfFrames = numOfFrames;
}

size_t NexusFileReader::findFrameNumberOfTime(float time) {
  auto frameNumber = static_cast<size_t>(std::floor(time / 0.1));
  return frameNumber;
}

std::unordered_map<hsize_t, sEEventVector> NexusFileReader::getSEEventMap() {
  std::unordered_map<hsize_t, sEEventVector> sEEventMap;

  auto groupNames = getNamesInGroup("/raw_data_1/selog");
  for (auto const name : groupNames) {
    if (name != "SECI_OUT_OF_RANGE_BLOCK") {
      std::vector<float> floatValues;
      std::vector<int32_t> intValues;
      std::vector<int64_t> longValues;
      std::vector<std::string> stringValues;
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
        stringValues = get1DStringDataset(valueDatasetName);
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
                std::make_shared<SampleEnvironmentEventDouble>(name, times[i],
                                                               floatValues[i]));
          else if (valueType == PredType::NATIVE_INT32)
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventInt>(name, times[i],
                                                            intValues[i]));
          else if (valueType == PredType::NATIVE_INT64)
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventLong>(name, times[i],
                                                             longValues[i]));
          else
            sEEventMap[frameNumber].push_back(
                std::make_shared<SampleEnvironmentEventString>(
                    name, times[i], stringValues[i]));
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
  values.resize(dataspace.getSelectNpoints());

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
int32_t NexusFileReader::getPeriodNumber() {
  DataSet dataset = m_file->openDataSet("/raw_data_1/periods/number");
  int32_t periodNumber;
  dataset.read(&periodNumber, PredType::NATIVE_INT32);

  return periodNumber;
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
 * Get the start time of the run as a Unix timestamp
 *
 * @return - Unix timestamp of start of run
 */
int64_t NexusFileReader::getRunStartTime() {
  DataSet dataset = m_file->openDataSet("/raw_data_1/start_time");
  std::string startTime;
  dataset.read(startTime, dataset.getDataType(), dataset.getSpace());
  return convertStringToUnixTime(startTime);
}

int64_t
NexusFileReader::convertStringToUnixTime(const std::string &timeString) {
  std::tm tmb = {};
#if defined(__GNUC__) && __GNUC__ >= 5
  std::istringstream ss(timeString);
  ss.imbue(std::locale("en_GB.utf-8"));
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
#else
  // gcc < 5 does not have std::get_time implemented
  strptime(timeString.c_str(), "%Y-%m-%dT%H:%M:%S", &tmb);
#endif
  auto timeUnix = std::mktime(&tmb);
  return static_cast<int64_t>(timeUnix);
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
 * Gets the event index of the start of the specified frame
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - event index corresponding to the start of the specified frame
 */
double NexusFileReader::getFrameTime(hsize_t frameNumber) {
  std::string datasetName = "/raw_data_1/detector_1_events/event_time_zero";

  auto frameTime = getSingleValueFromDataset<double>(
      datasetName, PredType::NATIVE_DOUBLE, frameNumber);

  return frameTime;
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
bool NexusFileReader::getEventDetIds(std::vector<int32_t> &detIds,
                                     hsize_t frameNumber) {
  if (frameNumber >= m_numberOfFrames)
    return false;
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

  dataset.read(detIds.data(), PredType::NATIVE_INT32, memspace, dataspace);

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
bool NexusFileReader::getEventTofs(std::vector<float> &tofs,
                                   hsize_t frameNumber) {
  if (frameNumber >= m_numberOfFrames)
    return false;
  auto dataset =
      m_file->openDataSet("/raw_data_1/detector_1_events/event_time_offset");

  auto numberOfEventsInFrame = getNumberOfEventsInFrame(frameNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber);
  hsize_t stride = 1;
  hsize_t block = 1;

  auto dataspace = dataset.getSpace();
  dataspace.selectHyperslab(H5S_SELECT_SET, &count, &offset, &stride, &block);

  tofs.resize(numberOfEventsInFrame);

  hsize_t dimsm = numberOfEventsInFrame;
  DataSpace memspace(1, &dimsm);

  dataset.read(tofs.data(), PredType::NATIVE_FLOAT, memspace, dataspace);

  return true;
}

std::vector<int>
NexusFileReader::getFramePartsPerFrame(int maxEventsPerMessage) {
  std::vector<int> framePartsPerFrame;
  framePartsPerFrame.resize(m_numberOfFrames);
  for (hsize_t frameNumber = 0; frameNumber < m_numberOfFrames; frameNumber++) {
    framePartsPerFrame[frameNumber] = static_cast<int>(
        std::ceil(static_cast<float>(getNumberOfEventsInFrame(frameNumber)) /
                  static_cast<float>(maxEventsPerMessage)));
  }
  return framePartsPerFrame;
}
