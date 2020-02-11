#include <6s4t_run_stop_generated.h>
#include <pl72_run_start_generated.h>
#include <sstream>

#include "DetectorSpectrumMapData.h"
#include "RunData.h"

namespace {
uint64_t secondsToNanoseconds(time_t timeInSeconds) {
  uint64_t timeInNanoseconds =
      static_cast<uint64_t>(timeInSeconds) * 1000000000L;
  return timeInNanoseconds;
}

uint64_t timeStringToUint64(const std::string &inputTime) {
  std::tm tmb = {};
  std::istringstream ss(inputTime);
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
#if (defined(_MSC_VER))
#define timegm _mkgmtime
#endif
  auto nsSinceEpoch = secondsToNanoseconds(timegm(&tmb));
  return nsSinceEpoch;
}
}

void RunData::setStartTimeFromString(const std::string &inputTime) {
  startTime = timeStringToUint64(inputTime);
}

void RunData::setStartTimeInSeconds(time_t inputTime) {
  startTime = secondsToNanoseconds(inputTime);
}

void RunData::setStopTimeFromString(const std::string &inputTime) {
  stopTime = timeStringToUint64(inputTime);
}

Streamer::Message serialiseRunStartMessage(
    const RunData &runData,
    const nonstd::optional<DetectorSpectrumMapData> &detSpecMap) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(runData.instrumentName);
  auto runID = builder.CreateString(runData.runID);
  auto nexusStructure = builder.CreateString(runData.nexusStructure);
  auto jobID = builder.CreateString(runData.jobID);
  auto serviceID = builder.CreateString(runData.serviceID);
  auto broker = builder.CreateString(runData.broker);
  auto filename = builder.CreateString(runData.filename);

  flatbuffers::Offset<RunStart> messageRunStart;
  if (detSpecMap) {
    auto detectorSpectrumMap = *detSpecMap;
    messageRunStart = CreateRunStart(
        builder, runData.startTime, runData.stopTime, runID, instrumentName,
        nexusStructure, jobID, broker, serviceID, filename,
        runData.numberOfPeriods, detectorSpectrumMap.addToBuffer(builder));
  } else {
    messageRunStart =
        CreateRunStart(builder, runData.startTime, runData.stopTime, runID,
                       instrumentName, nexusStructure, jobID, broker, serviceID,
                       filename, runData.numberOfPeriods);
  }

  FinishRunStartBuffer(builder, messageRunStart);

  return Streamer::Message(builder.Release());
}

Streamer::Message serialiseRunStopMessage(const RunData &runData) {
  flatbuffers::FlatBufferBuilder builder;

  auto runID = builder.CreateString(runData.runID);
  auto jobID = builder.CreateString(runData.jobID);
  auto serviceID = builder.CreateString(runData.serviceID);

  auto messageRunStop =
      CreateRunStop(builder, runData.stopTime, runID, jobID, serviceID);
  FinishRunStopBuffer(builder, messageRunStop);

  return Streamer::Message(builder.Release());
}

RunData deserialiseRunStartMessage(const uint8_t *buffer) {
  auto runData{RunData()};
  auto runStartData = GetRunStart(buffer);
  runData.startTime = runStartData->start_time();
  runData.stopTime = runStartData->stop_time();
  runData.runID = runStartData->run_name()->str();
  runData.instrumentName = runStartData->instrument_name()->str();
  runData.nexusStructure = runStartData->nexus_structure()->str();
  runData.jobID = runStartData->job_id()->str();
  runData.serviceID = runStartData->service_id()->str();
  runData.numberOfPeriods = runStartData->n_periods();
  runData.broker = runStartData->broker()->str();
  runData.filename = runStartData->filename()->str();

  return runData;
}

RunData deserialiseRunStopMessage(const uint8_t *buffer) {
  auto runData{RunData()};
  auto runStopData = GetRunStop(buffer);
  runData.stopTime = runStopData->stop_time();
  runData.runID = runStopData->run_name()->str();
  runData.jobID = runStopData->job_id()->str();
  runData.serviceID = runStopData->service_id()->str();

  return runData;
}
