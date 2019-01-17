#include "RunData.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

void RunData::setStartTime(const std::string &inputTime) {
  m_startTime = timeStringToUint64(inputTime);
}

void RunData::setStartTimeInSeconds(time_t inputTime) {
  m_startTime = secondsToNanoseconds(inputTime);
}

void RunData::setStopTime(const std::string &inputTime) {
  m_stopTime = timeStringToUint64(inputTime);
}

uint64_t RunData::secondsToNanoseconds(time_t timeInSeconds) {
  uint64_t timeInNanoseconds =
      static_cast<uint64_t>(timeInSeconds) * 1000000000L;
  return timeInNanoseconds;
}

uint64_t RunData::timeStringToUint64(const std::string &inputTime) {
  std::tm tmb = {};
  std::istringstream ss(inputTime);
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
#if (defined(_MSC_VER))
#define timegm _mkgmtime
#endif
  auto nsSinceEpoch = secondsToNanoseconds(timegm(&tmb));
  return nsSinceEpoch;
}

bool RunData::decodeMessage(const uint8_t *buf) {
  auto runData = GetRunInfo(buf);

  if (runData->info_type_type() == InfoTypes::RunStart) {
    auto runStartData = static_cast<const RunStart *>(runData->info_type());
    setStartTimeInNanoseconds(runStartData->start_time());
    setInstrumentName(runStartData->instrument_name()->str());
    setRunNumber(runStartData->run_number());
    setNumberOfPeriods(runStartData->n_periods());

    return true;
  }
  if (runData->info_type_type() == InfoTypes::RunStop) {
    auto runStopData = static_cast<const RunStop *>(runData->info_type());
    setStopTime(runStopData->stop_time());

    return true;
  }

  return false; // this is not a RunData message
}

Streamer::Message RunData::getRunStartBuffer() {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunStart = CreateRunStart(builder, m_startTime, m_runNumber,
                                        instrumentName, m_numberOfPeriods);
  auto messageRunInfo =
      CreateRunInfo(builder, InfoTypes::RunStart, messageRunStart.Union());

  FinishRunInfoBuffer(builder, messageRunInfo);

  return Streamer::Message(builder.Release());
}

Streamer::Message RunData::getRunStopBuffer() {
  flatbuffers::FlatBufferBuilder builder;

  auto messageRunStop = CreateRunStop(builder, m_stopTime);
  auto messageRunInfo =
      CreateRunInfo(builder, InfoTypes::RunStop, messageRunStop.Union());

  FinishRunInfoBuffer(builder, messageRunInfo);

  return Streamer::Message(builder.Release());
}

std::string RunData::runInfo() {
  std::stringstream ssRunInfo;
  ssRunInfo.imbue(std::locale());
  ssRunInfo << "Run number: " << m_runNumber << ", "
            << "Instrument name: " << m_instrumentName << ", "
            << "Start time: ";
  // convert nanoseconds to seconds
  const auto sTime = static_cast<time_t>(m_startTime / 1000000000);
  ssRunInfo << std::put_time(std::gmtime(&sTime), "%Y-%m-%dT%H:%M:%S");
  return ssRunInfo.str();
}
