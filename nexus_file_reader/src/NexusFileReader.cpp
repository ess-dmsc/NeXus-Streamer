#include "../include/NexusFileReader.h"
#include "../../event_data/include/SampleEnvironmentEventDouble.h"
#include "../../event_data/include/SampleEnvironmentEventInt.h"
#include "../../event_data/include/SampleEnvironmentEventLong.h"
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

/**
 * Create a object to read the specified file
 *
 * @param filename - the full path of the NeXus file
 * @return - an object with which to read information from the file
 */
NexusFileReader::NexusFileReader(hdf5::file::File file, uint64_t runStartTime,
                                 const int32_t fakeEventsPerPulse,
                                 const std::vector<int32_t> &detectorNumbers)
    : m_file(std::move(file)), m_runStart(runStartTime),
      m_fakeEventsPerPulse(fakeEventsPerPulse),
      m_timeOfFlightDist(10000, 100000), m_detectorNumbers(detectorNumbers),
      m_detectorIDDist(0, static_cast<uint32_t>(detectorNumbers.size() - 1)) {
  if (!m_file.is_valid()) {
    throw std::runtime_error("Failed to open specified NeXus file");
  }
  getEntryGroup(m_file.root(), m_entryGroup);
  getEventGroup(m_entryGroup, m_eventGroup);

  auto dataset = m_eventGroup.get_dataset("event_time_zero");
  m_numberOfFrames = static_cast<size_t>(dataset.dataspace().size());
  // Use pulse times relative to start time rather than using the `offset`
  // attribute from the NeXus file, this makes the timestamps look as if this
  // data is coming from a live instrument
  m_frameStartOffset = m_runStart;
}

void NexusFileReader::getEntryGroup(const hdf5::node::Group &rootGroup,
                                    hdf5::node::Group &entryGroupOutput) {
  for (const auto &rootChild : rootGroup.nodes) {
    if (rootChild.attributes.exists("NX_class")) {
      auto attr = rootChild.attributes["NX_class"];
      std::string nxClassType;
      attr.read(nxClassType, attr.datatype());
      if (nxClassType == "NXentry") {
        entryGroupOutput = rootChild;
        return;
      }
    }
  }
  throw std::runtime_error(
      "Failed to find an NXentry group in the NeXus file root");
}

void NexusFileReader::getEventGroup(const hdf5::node::Group &entryGroup,
                                    hdf5::node::Group &eventGroupOutput) {
  for (const auto &entryChild : entryGroup.nodes) {
    if (entryChild.attributes.exists("NX_class")) {
      auto attr = entryChild.attributes["NX_class"];
      std::string nxClassType;
      attr.read(nxClassType, attr.datatype());
      if (nxClassType == "NXevent_data") {
        eventGroupOutput = entryChild;
        checkEventGroupHasRequiredDatasets(eventGroupOutput);
        return;
      }
    }
  }
  throw std::runtime_error("Required NXevent_data group missing "
                           "from the NXentry group");
}

void NexusFileReader::checkEventGroupHasRequiredDatasets(
    const hdf5::node::Group &eventGroup) const {
  std::vector<std::string> requiredDatasets = {
      "event_time_zero", "event_time_offset", "event_id", "event_index"};
  for (const auto &datasetName : requiredDatasets) {
    if (!eventGroup.has_dataset(datasetName)) {
      throw std::runtime_error("Required dataset \"" + datasetName +
                               "\" missing "
                               "from the NXevent_data group");
    }
  }
}

size_t NexusFileReader::findFrameNumberOfTime(float time) {
  auto frameNumber = static_cast<size_t>(std::floor(time / 0.1));
  return frameNumber;
}

std::unordered_map<hsize_t, sEEventVector> NexusFileReader::getSEEventMap() {
  std::unordered_map<hsize_t, sEEventVector> sEEventMap;

  if (!m_entryGroup.has_group("selog")) {
    std::cout << "Warning: no selog group found, not publishing sample "
                 "environment log data\n";
    return sEEventMap;
  }

  auto sampleEnvGroup = m_entryGroup.get_group("selog");
  for (auto const &sampleEnvChild : sampleEnvGroup.nodes) {
    hdf5::node::Group logGroup;
    hdf5::node::Group valueLog;
    if (sampleEnvChild.type() == hdf5::node::Type::GROUP) {
      logGroup = static_cast<hdf5::node::Group>(sampleEnvChild);
      if (!logGroup.exists("value_log"))
        continue;
      valueLog = logGroup.get_group("value_log");
      if (!valueLog.exists("time") || !valueLog.exists("value"))
        continue;
    } else {
      continue;
    }
    std::vector<float> times;
    std::vector<float> floatValues;
    std::vector<int32_t> intValues;
    std::vector<int64_t> longValues;

    const auto floatType = hdf5::datatype::create<float>();
    const auto int32Type = hdf5::datatype::create<int32_t>();
    const auto int64Type = hdf5::datatype::create<int64_t>();

    auto timeDataset = valueLog.get_dataset("time");
    times.resize(static_cast<size_t>(timeDataset.dataspace().size()));
    timeDataset.read(times);

    std::string name = logGroup.link().target().object_path().name();

    auto valueDataset = valueLog.get_dataset("value");
    auto valueType = valueDataset.datatype();
    auto dataSize = static_cast<size_t>(valueDataset.dataspace().size());
    if (valueType == floatType) {
      floatValues.resize(dataSize);
      valueDataset.read(floatValues);
    } else if (valueType == int32Type) {
      intValues.resize(dataSize);
      valueDataset.read(intValues);
    } else if (valueType == int64Type) {
      longValues.resize(dataSize);
      valueDataset.read(longValues);
    } else {
      std::cout << "Unsupported datatype found in log dataset " << name << "\n";
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
        if (valueType == floatType)
          sEEventMap[frameNumber].push_back(
              std::make_shared<SampleEnvironmentEventDouble>(
                  name, times[i], floatValues[i], m_runStart));
        else if (valueType == int32Type)
          sEEventMap[frameNumber].push_back(
              std::make_shared<SampleEnvironmentEventInt>(
                  name, times[i], intValues[i], m_runStart));
        else if (valueType == int64Type)
          sEEventMap[frameNumber].push_back(
              std::make_shared<SampleEnvironmentEventLong>(
                  name, times[i], longValues[i], m_runStart));
      }
    }
  }
  return sEEventMap;
}

/**
 * Get the size of the NeXus file in bytes
 *
 * @return - the size of the file in bytes
 */
hsize_t NexusFileReader::getFileSize() { return m_file.size(); }

/**
 * Get the total number of events in the file
 *
 * @return - total number of events
 */
uint64_t NexusFileReader::getTotalEventCount() {
  if (m_fakeEventsPerPulse > 0) {
    return getNumberOfFrames() * m_fakeEventsPerPulse;
  }

  auto dataset = m_eventGroup.get_dataset("event_time_offset");
  return static_cast<uint64_t>(dataset.dataspace().size());
}

uint32_t NexusFileReader::getPeriodNumber() { return 0; }

int32_t NexusFileReader::getNumberOfPeriods() { return 1; }

/**
 * Get instrument name
 *
 * @return - instrument name
 */
std::string NexusFileReader::getInstrumentName() {
  auto dataset = m_entryGroup.get_dataset("name");
  std::string instrumentName;
  dataset.read(instrumentName, dataset.datatype(), dataset.dataspace());
  return instrumentName;
}

/**
 * Get the proton charge
 *
 * @return - the proton charge
 */
float NexusFileReader::getProtonCharge(hsize_t frameNumber) {
  std::string datasetName = "framelog/proton_charge/value";
  if (m_entryGroup.has_dataset(datasetName)) {

    auto protonCharge = getSingleValueFromDataset<float>(
        m_entryGroup, datasetName, frameNumber);

    return protonCharge;
  }
  return -1;
}

/**
 * Gets the absolute time of the start of the specified frame
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - absolute time of frame start in nanoseconds since 1 Jan 1970
 */
uint64_t NexusFileReader::getFrameTime(hsize_t frameNumber) {
  std::string datasetName = "event_time_zero";

  auto frameTime =
      getSingleValueFromDataset<double>(m_eventGroup, datasetName, frameNumber);
  auto frameTimeFromOffsetNanoseconds =
      static_cast<uint64_t>(floor((frameTime * 1e9) + 0.5));
  return m_frameStartOffset + frameTimeFromOffsetNanoseconds;
}

template <typename T>
T NexusFileReader::getSingleValueFromDataset(const hdf5::node::Group &group,
                                             const std::string &datasetName,
                                             hsize_t offset) {
  auto dataset = group.get_dataset(datasetName);
  T value;

  m_slab.offset({offset});

  dataset.read(value, m_slab);

  return value;
}

/**
 * Gets the event index of the start of the specified frame
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - event index corresponding to the start of the specified frame
 */
hsize_t NexusFileReader::getFrameStart(hsize_t frameNumber) {
  std::string datasetName = "event_index";

  auto frameStart = getSingleValueFromDataset<hsize_t>(
      m_eventGroup, datasetName, frameNumber);

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

  auto dataset = m_eventGroup.get_dataset("event_id");

  auto numberOfEventsInFrame = getNumberOfEventsInFrame(frameNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber);
  detIds.resize(count);

  auto slab = hdf5::dataspace::Hyperslab({offset}, {count}, {1});

  dataset.read(detIds, slab);

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

  auto dataset = m_eventGroup.get_dataset("event_time_offset");

  auto numberOfEventsInFrame = getNumberOfEventsInFrame(frameNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber);

  auto slab = hdf5::dataspace::Hyperslab({offset}, {count}, {1});
  std::vector<float> tof_floats(count);
  tofs.resize(numberOfEventsInFrame);

  dataset.read(tof_floats, slab);
  // transform float in microseconds to uint32 in nanoseconds
  std::transform(tof_floats.begin(), tof_floats.end(), tofs.begin(),
                 [](float tof) {
                   return static_cast<uint32_t>(floor((tof * 1000) + 0.5));
                 });

  return true;
}
