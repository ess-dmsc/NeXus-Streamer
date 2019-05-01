#include "../include/NexusFileReader.h"
#include "../../core/include/EventDataFrame.h"
#include "../../core/include/HistogramFrame.h"
#include "../../serialisation/include/SampleEnvironmentEventDouble.h"
#include "../../serialisation/include/SampleEnvironmentEventInt.h"
#include "../../serialisation/include/SampleEnvironmentEventLong.h"
#include <fmt/format.h>

namespace {
uint64_t secondsToNanoseconds(double seconds) {
  return static_cast<uint64_t>(round(seconds * 1000000000LL));
}

uint64_t secondsToMilliseconds(double seconds) {
  return static_cast<uint64_t>(round(seconds * 1000LL));
}

uint64_t nanosecondsToMilliseconds(uint64_t nanoseconds) {
  return nanoseconds / 1000000LL;
}

std::vector<uint64_t> secondsToNanoseconds(std::vector<double> const &seconds) {
  std::vector<uint64_t> nanoseconds;
  std::transform(seconds.cbegin(), seconds.cend(),
                 std::back_inserter(nanoseconds),
                 [](double const secondsValue) {
                   return secondsToNanoseconds(secondsValue);
                 });
  return nanoseconds;
}

/**
 * We can only currently deal with multiple NXevent_data groups if they contain
 * exactly the same frames
 * If this is not the case then throw an error
 *
 * @param eventGroups - the NXevent_data groups found in the file
 */
void checkEventDataGroupsHaveConsistentFrames(
    std::vector<hdf5::node::Group> const &eventGroups) {
  if (eventGroups.size() > 1) {
    auto firstGroupPulseDataset = eventGroups[0].get_dataset("event_time_zero");
    std::vector<double> firstGroupPulseTimes(
        static_cast<size_t>(firstGroupPulseDataset.dataspace().size()));
    firstGroupPulseDataset.read(firstGroupPulseTimes);
    auto firstPulseTimesNs = secondsToNanoseconds(firstGroupPulseTimes);
    for (auto const &eventGroup : eventGroups) {
      auto pulseTimesDataset = eventGroup.get_dataset("event_time_zero");
      std::vector<double> pulseTimes(
          static_cast<size_t>(pulseTimesDataset.dataspace().size()));
      pulseTimesDataset.read(pulseTimes);
      auto pulseTimesNs = secondsToNanoseconds(pulseTimes);
      if (firstPulseTimesNs != pulseTimesNs) {
        throw std::runtime_error("NXevent_data groups in the file do not "
                                 "contain the same frames as each other, this "
                                 "is not currently supported.");
      }
    }
  }
}
}

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
      m_detectorNumbers(detectorNumbers), m_timeOfFlightDist(10000, 100000),
      m_detectorIDDist(0, static_cast<uint32_t>(detectorNumbers.size() - 1)) {
  if (!m_file.is_valid()) {
    throw std::runtime_error("Failed to open specified NeXus file");
  }
  getEntryGroup(m_file.root(), m_entryGroup);
  getGroups(
      m_entryGroup, m_eventGroups, "NXevent_data",
      {"event_time_zero", "event_time_offset", "event_id", "event_index"});
  getGroups(m_entryGroup, m_histoGroups, "NXdata",
            {"time_of_flight", "counts"});

  if (m_eventGroups.empty() && m_histoGroups.empty()) {
    throw std::runtime_error(
        "No valid NXevent_data or NXdata groups found in the NXentry group");
  }

  m_isisFile = testIfIsISISFile();

  try {
    checkEventDataGroupsHaveConsistentFrames(m_eventGroups);
  } catch (const std::runtime_error &e) {
    m_logger->warn(e.what());
    m_logger->warn(
        "Only data from one NXevent_data group will be published to Kafka: {}",
        m_eventGroups[0].link().path().name());
    m_eventGroups.resize(1);
  }

  if (m_eventGroups.empty()) {
    m_numberOfFrames = 0;
  } else {
    auto frameTimes = m_eventGroups[0].get_dataset("event_time_zero");
    m_numberOfFrames = static_cast<size_t>(frameTimes.dataspace().size());
  }
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

void NexusFileReader::getGroups(
    const hdf5::node::Group &entryGroup,
    std::vector<hdf5::node::Group> &groupsOutput, const std::string &className,
    const std::vector<std::string> &requiredDatasets) {
  for (const auto &entryChild : entryGroup.nodes) {
    if (entryChild.attributes.exists("NX_class")) {
      auto attr = entryChild.attributes["NX_class"];
      std::string nxClassType;
      attr.read(nxClassType, attr.datatype());
      if (nxClassType == className) {
        try {
          checkGroupHasRequiredDatasets(entryChild, requiredDatasets,
                                        className);
          groupsOutput.emplace_back(entryChild);
        } catch (const std::runtime_error &e) {
          m_logger->warn(e.what());
        }
      }
    }
  }
}

void NexusFileReader::checkGroupHasRequiredDatasets(
    const hdf5::node::Group &group,
    const std::vector<std::string> &requiredDatasets,
    const std::string &className) const {
  for (const auto &datasetName : requiredDatasets) {
    if (!group.has_dataset(datasetName)) {
      throw std::runtime_error(
          fmt::format("Required dataset {} missing from {} group at {}",
                      datasetName, className, group.link().path().name()));
    }
  }
}

size_t NexusFileReader::findFrameNumberOfTime(float time) {
  auto frameNumber = static_cast<size_t>(std::floor(time / 0.1));
  return frameNumber;
}

std::vector<hdf5::node::Group> NexusFileReader::findNXLogs() {
  std::vector<hdf5::node::Group> NXlogs;
  std::for_each(hdf5::node::RecursiveNodeIterator::begin(m_entryGroup),
                hdf5::node::RecursiveNodeIterator::end(m_entryGroup),
                [&NXlogs](const hdf5::node::Node &node) {
                  if (node.type() == hdf5::node::Type::GROUP) {
                    std::string NXClassValue;
                    if (node.attributes.exists("NX_class")) {
                      node.attributes["NX_class"].read(NXClassValue);
                      if (NXClassValue == "NXlog") {
                        NXlogs.emplace_back(node);
                      }
                    }
                  }
                });

  return NXlogs;
}

std::unordered_map<hsize_t, sEEventVector> NexusFileReader::getSEEventMap() {
  if (m_eventGroups.empty()) {
    m_logger->warn("NeXus-Streamer does not currently support streaming NXlog "
                   "data in the case that there is no NXevent_data group in "
                   "the file. Please create an issue on github if this feature "
                   "would be useful to you.");
    return {};
  }

  std::unordered_map<hsize_t, sEEventVector> sEEventMap;
  auto NXlogs = findNXLogs();

  if (NXlogs.empty()) {
    m_logger->warn(
        "No NXlog groups found, not publishing sample environment log data");
    return sEEventMap;
  }

  for (auto const &sampleEnvGroup : NXlogs) {
    if (!sampleEnvGroup.exists("time") || !sampleEnvGroup.exists("value"))
      continue;
    std::vector<float> times;
    std::vector<float> floatValues;
    std::vector<int32_t> intValues;
    std::vector<int64_t> longValues;

    const auto floatType = hdf5::datatype::create<float>();
    const auto int32Type = hdf5::datatype::create<int32_t>();
    const auto int64Type = hdf5::datatype::create<int64_t>();

    auto timeDataset = sampleEnvGroup.get_dataset("time");
    times.resize(static_cast<size_t>(timeDataset.dataspace().size()));
    timeDataset.read(times);

    std::string name = sampleEnvGroup.link().target().object_path().name();

    // For ISIS files if the log is called value_log then we should instead use
    // the name of the parent object
    if (isISISFile() && name == "value_log") {
      name =
          sampleEnvGroup.link().parent().link().target().object_path().name();
    }

    auto valueDataset = sampleEnvGroup.get_dataset("value");
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
      m_logger->warn("Unsupported datatype found in log dataset {}", name);
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
    return getNumberOfFrames() * m_fakeEventsPerPulse * m_eventGroups.size();
  }

  uint64_t totalEvents = 0;
  for (auto const &eventGroup : m_eventGroups) {
    auto dataset = eventGroup.get_dataset("event_time_offset");
    totalEvents += static_cast<uint64_t>(dataset.dataspace().size());
  }
  return totalEvents;
}

/**
 * Get the total number of events in the event data group
 *
 * @return - total number of events
 */
uint64_t NexusFileReader::getTotalEventsInGroup(size_t eventGroupNumber) {
  if (m_fakeEventsPerPulse > 0) {
    return getNumberOfFrames() * m_fakeEventsPerPulse;
  }

  auto dataset =
      m_eventGroups[eventGroupNumber].get_dataset("event_time_offset");
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

  auto dataset = m_eventGroups[0].get_dataset(datasetName);
  std::string units;
  uint64_t frameTimeFromOffsetNanoseconds;
  if (dataset.attributes.exists("units")) {
    dataset.attributes["units"].read(units);
    if (units == "ns" || units == "nanoseconds") {
      frameTimeFromOffsetNanoseconds = getSingleValueFromDataset<uint64_t>(
          m_eventGroups[0], datasetName, frameNumber);
      return m_frameStartOffset + frameTimeFromOffsetNanoseconds;
    }
    // else assume seconds
  }
  auto frameTime = getSingleValueFromDataset<double>(m_eventGroups[0],
                                                     datasetName, frameNumber);
  frameTimeFromOffsetNanoseconds = secondsToNanoseconds(frameTime);
  return m_frameStartOffset + frameTimeFromOffsetNanoseconds;
}

/**
 * Gets the frame time relative to the start of run, in milliseconds
 *
 * @param frameNumber - find the event index for the start of this frame
 * @return - relative time of frame in milliseconds since run start
 */
uint64_t
NexusFileReader::getRelativeFrameTimeMilliseconds(const hsize_t frameNumber) {
  std::string datasetName = "event_time_zero";

  auto dataset = m_eventGroups[0].get_dataset(datasetName);
  std::string units;
  if (dataset.attributes.exists("units")) {
    dataset.attributes["units"].read(units);
    if (units == "ns" || units == "nanoseconds") {
      auto frameTimeNanoseconds = getSingleValueFromDataset<uint64_t>(
          m_eventGroups[0], datasetName, frameNumber);
      return nanosecondsToMilliseconds(frameTimeNanoseconds);
    }
    // else assume seconds
  }
  auto frameTime = getSingleValueFromDataset<double>(m_eventGroups[0],
                                                     datasetName, frameNumber);
  return secondsToMilliseconds(frameTime);
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
hsize_t NexusFileReader::getFrameStart(hsize_t frameNumber,
                                       size_t eventGroupNumber) {
  std::string datasetName = "event_index";
  auto frameStart = getSingleValueFromDataset<hsize_t>(
      m_eventGroups[eventGroupNumber], datasetName, frameNumber);

  return frameStart;
}

/**
 * Get the number of events which are in the specified frame
 *
 * @param frameNumber - the number of the frame in which to count the number of
 * events
 * @return - the number of events in the specified frame
 */
hsize_t NexusFileReader::getNumberOfEventsInFrame(hsize_t frameNumber,
                                                  size_t eventGroupNumber) {
  if (m_fakeEventsPerPulse > 0) {
    return static_cast<hsize_t>(m_fakeEventsPerPulse);
  }
  // if this is the last frame then we cannot get number of events by looking at
  // event index of next frame
  if (frameNumber == (m_numberOfFrames - 1)) {
    return getTotalEventsInGroup(eventGroupNumber) -
           getFrameStart(frameNumber, eventGroupNumber);
  }
  return getFrameStart(frameNumber + 1, eventGroupNumber) -
         getFrameStart(frameNumber, eventGroupNumber);
}

/**
 * Get the list of detector IDs corresponding to events in the specified frame
 *
 * @param frameNumber - the number of the frame in which to get the detector IDs
 * @return - vector of detIds, empty if the specified frame number is not the
 * data range
 */
std::vector<uint32_t> NexusFileReader::getEventDetIds(hsize_t frameNumber,
                                                      size_t eventGroupNumber) {
  if (frameNumber >= m_numberOfFrames)
    return {};

  std::vector<uint32_t> detIds;

  if (m_fakeEventsPerPulse > 0) {
    detIds.reserve(static_cast<size_t>(m_fakeEventsPerPulse));
    for (size_t i = 0; i < static_cast<size_t>(m_fakeEventsPerPulse); i++) {
      detIds.push_back(static_cast<uint32_t>(
          m_detectorNumbers[m_detectorIDDist(RandomEngine)]));
    }
    return detIds;
  }

  auto dataset = m_eventGroups[eventGroupNumber].get_dataset("event_id");

  auto numberOfEventsInFrame =
      getNumberOfEventsInFrame(frameNumber, eventGroupNumber);

  hsize_t offset = getFrameStart(frameNumber, eventGroupNumber);
  detIds.resize(numberOfEventsInFrame);

  auto slab =
      hdf5::dataspace::Hyperslab({offset}, {numberOfEventsInFrame}, {1});

  dataset.read(detIds, slab);

  return detIds;
}

/**
 * Get the list of flight times corresponding to events in the specifed frame
 *
 * @param frameNumber - the number of the frame in which to get the
 * time-of-flights
 * @return - vector of flight times, empty if the specified frame number is not
 * the data range
 */
std::vector<uint32_t> NexusFileReader::getEventTofs(hsize_t frameNumber,
                                                    size_t eventGroupNumber) {
  if (frameNumber >= m_numberOfFrames)
    return {};

  std::vector<uint32_t> tofs;

  if (m_fakeEventsPerPulse > 0) {
    tofs.reserve(static_cast<size_t>(m_fakeEventsPerPulse));
    for (size_t i = 0; i < static_cast<size_t>(m_fakeEventsPerPulse); i++) {
      tofs.push_back(static_cast<uint32_t>(m_timeOfFlightDist(RandomEngine)));
    }
    return tofs;
  }

  auto dataset =
      m_eventGroups[eventGroupNumber].get_dataset("event_time_offset");

  auto numberOfEventsInFrame =
      getNumberOfEventsInFrame(frameNumber, eventGroupNumber);

  hsize_t count = numberOfEventsInFrame;
  hsize_t offset = getFrameStart(frameNumber, eventGroupNumber);

  auto slab = hdf5::dataspace::Hyperslab({offset}, {count}, {1});
  std::vector<float> tof_floats(count);
  tofs.resize(numberOfEventsInFrame);

  dataset.read(tof_floats, slab);
  // transform float in microseconds to uint32 in nanoseconds
  std::transform(tof_floats.begin(), tof_floats.end(), tofs.begin(),
                 [](float tof) {
                   return static_cast<uint32_t>(floor((tof * 1000) + 0.5));
                 });

  return tofs;
}

std::vector<EventDataFrame> NexusFileReader::getEventData(hsize_t frameNumber) {
  std::vector<EventDataFrame> eventData;
  for (size_t eventGroupNumber = 0; eventGroupNumber < m_eventGroups.size();
       ++eventGroupNumber) {
    auto detIDs = getEventDetIds(frameNumber, eventGroupNumber);
    auto tofs = getEventTofs(frameNumber, eventGroupNumber);
    if (!detIDs.empty() && !tofs.empty()) {
      eventData.emplace_back(detIDs, tofs);
    }
  }
  return eventData;
}

std::vector<HistogramFrame> NexusFileReader::getHistoData() {
  std::vector<HistogramFrame> histogramData;
  for (const auto &histoGroup : m_histoGroups) {
    auto countsDataset = histoGroup.get_dataset("counts");
    std::vector<int32_t> counts(
        static_cast<size_t>(countsDataset.dataspace().size()));
    countsDataset.read(counts);
    hdf5::dataspace::Simple dataspace(countsDataset.dataspace());
    auto dims = dataspace.current_dimensions();
    std::vector<size_t> countsShape(dims.cbegin(), dims.cend());

    auto tofDataset = histoGroup.get_dataset("time_of_flight");
    std::vector<float> timeOfFlight(
        static_cast<size_t>(tofDataset.dataspace().size()));
    tofDataset.read(timeOfFlight);

    auto detIdsDataset = histoGroup.get_dataset("spectrum_index");
    std::vector<int32_t> detIds(
        static_cast<size_t>(detIdsDataset.dataspace().size()));
    detIdsDataset.read(detIds);

    histogramData.emplace_back(counts, countsShape, timeOfFlight, detIds);
  }
  return histogramData;
}

bool NexusFileReader::isISISFile() { return m_isisFile; }

/**
 * If the entry group is called "raw_data_1" and it contains a group called
 * "isis_vms_compat" then assume this file is from ISIS
 * This is consistent with how Mantid tests for ISIS files.
 *
 * @return - true if input file is from ISIS
 */
bool NexusFileReader::testIfIsISISFile() {
  if (m_file.root().has_group("raw_data_1")) {
    return m_entryGroup.has_group("isis_vms_compat");
  }
  return false;
}

uint32_t NexusFileReader::getRunDurationMs() {
  if (m_entryGroup.has_dataset("duration")) {
    auto durationDataset = m_entryGroup.get_dataset("duration");
    float duration;
    durationDataset.read(duration);
    auto durationInMs = static_cast<uint32_t>(duration * 1000);

    std::string units;
    if (durationDataset.attributes.exists("units")) {
      durationDataset.attributes["units"].read(units);
    }
    if (units != "s" && units != "second" && units != "seconds") {
      throw std::runtime_error(
          "duration dataset found but does not have units of seconds");
    }

    return durationInMs;
  }
  throw std::runtime_error("Unable to get run duration from file, no duration "
                           "dataset or event data found in file.");
}
