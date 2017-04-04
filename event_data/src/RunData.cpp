#include "RunData.h"

#include <array>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

void RunData::setStartTime(const std::string &inputTime) {
  m_startTime = timeStringToUint64(inputTime);
}

void RunData::setStopTime(const std::string &inputTime) {
  m_stopTime = timeStringToUint64(inputTime);
}

uint64_t RunData::timeStringToUint64(const std::string &inputTime) {
  std::tm tmb = {};
#if (defined(__GNUC__) && __GNUC__ >= 5) || defined(_MSC_VER)
  std::istringstream ss(inputTime);
  ss.imbue(std::locale());
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
#else
  // gcc < 5 does not have std::get_time implemented
  strptime(inputTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tmb);
#endif
  return static_cast<uint64_t>(std::mktime(&tmb));
}

bool RunData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISStream::GetEventMessage(buf);
  if (messageData->message_type() == ISISStream::MessageTypes_RunInfo) {
    auto runData =
        static_cast<const ISISStream::RunInfo *>(messageData->message());

    if (runData->info_type_type() == ISISStream::InfoTypes_RunStart) {
      auto runStartData =
          static_cast<const ISISStream::RunStart *>(runData->info_type());
      setStartTime(runStartData->start_time());
      setInstrumentName(runStartData->inst_name()->str());
      setRunNumber(runStartData->run_number());
      setNumberOfPeriods(runStartData->n_periods());

      return true;
    } else if (runData->info_type_type() == ISISStream::InfoTypes_RunStop) {
      auto runStopData =
          static_cast<const ISISStream::RunStop *>(runData->info_type());
      setStopTime(runStopData->stop_time());

      return true;
    }
  }
  return false; // this is not a RunData message
}

flatbuffers::unique_ptr_t RunData::getEventBufferPointer(std::string &buffer,
                                                         uint64_t messageID) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunStart = ISISStream::CreateRunStart(
      builder, m_startTime, m_runNumber, instrumentName, m_numberOfPeriods);
  auto messageRunInfo = ISISStream::CreateRunInfo(
      builder, ISISStream::InfoTypes_RunStart, messageRunStart.Union());

  auto messageFlatbuf =
      ISISStream::CreateEventMessage(builder, ISISStream::MessageTypes_RunInfo,
                                     messageRunInfo.Union(), messageID);
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

flatbuffers::unique_ptr_t
RunData::getRunStartBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunStart = ISISStream::CreateRunStart(
      builder, m_startTime, m_runNumber, instrumentName, m_numberOfPeriods);
  auto messageRunInfo = ISISStream::CreateRunInfo(
      builder, ISISStream::InfoTypes_RunStart, messageRunStart.Union());

  builder.Finish(messageRunInfo);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

flatbuffers::unique_ptr_t
RunData::getRunStopBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto messageRunStop = ISISStream::CreateRunStop(builder, m_stopTime);
  auto messageRunInfo = ISISStream::CreateRunInfo(
      builder, ISISStream::InfoTypes_RunStop, messageRunStop.Union());

  builder.Finish(messageRunInfo);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

std::string RunData::runInfo() {
  std::stringstream ssRunInfo;
  ssRunInfo.imbue(std::locale());
  ssRunInfo << "Run number: " << m_runNumber << ", "
            << "Instrument name: " << m_instrumentName << ", "
            << "Start time: ";
  const time_t sTime = static_cast<time_t>(m_startTime);
#if defined(__GNUC__) && __GNUC__ >= 5
  ssRunInfo << std::put_time(std::gmtime(&sTime), "%Y-%m-%dT%H:%M:%S");
#else
  // gcc < 5 does not have std::put_time implemented
  auto tmb = std::gmtime(&sTime);
  std::array<char, 20> buffer;
  strftime(buffer.data(), 20, "%Y-%m-%dT%H:%M:%S", tmb);
  ssRunInfo << buffer.data();
#endif
  return ssRunInfo.str();
}
