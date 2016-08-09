#include "../include/NexusFileReader.h"
#include <iostream>

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
 * Get the proton charge
 *
 * @return - the proton charge
 */
float NexusFileReader::getProtonCharge() {
  DataSet dataset = m_file->openDataSet("/raw_data_1/proton_charge");
  float protonCharge;
  dataset.read(&protonCharge, PredType::NATIVE_FLOAT);

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
bool NexusFileReader::getEventDetIds(std::vector<uint32_t> &detIds,
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
bool NexusFileReader::getEventTofs(std::vector<uint64_t> &tofs,
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

  std::vector<double> timeOffsetArray(numberOfEventsInFrame);

  hsize_t dimsm = numberOfEventsInFrame;
  DataSpace memspace(1, &dimsm);

  dataset.read(timeOffsetArray.data(), PredType::NATIVE_DOUBLE, memspace,
               dataspace);

  tofs.resize(numberOfEventsInFrame);

  for (size_t tofIndex = 0; tofIndex < numberOfEventsInFrame; tofIndex++) {
    tofs[tofIndex] = static_cast<uint64_t>((timeOffsetArray[tofIndex] * 1e3));
  }

  return true;
}
