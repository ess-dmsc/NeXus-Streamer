#include "RunData.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

void RunData::setStartTime(const std::string &inputTime) {
  std::tm tmb = {};
#if defined(__GNUC__) && __GNUC__ >= 5
  std::istringstream ss(inputTime);
  ss.imbue(std::locale("en_GB.utf-8"));
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
#else
  // gcc < 5 does not have std::get_time implemented
  strptime(inputTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tmb);
#endif
  m_startTime = static_cast<uint64_t>(std::mktime(&tmb));
}

bool RunData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISDAE::GetEventMessage(buf);
  if (messageData->message_type() == ISISDAE::MessageTypes_RunInfo) {
    auto runData =
        static_cast<const ISISDAE::RunInfo *>(messageData->message());

    setStartTime(runData->start_time());
    setInstrumentName(runData->inst_name()->str());
    setRunNumber(runData->run_number());

    return true;
  }
  return false; // this is not a RunData message
}

flatbuffers::unique_ptr_t RunData::getEventBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunInfo = ISISDAE::CreateRunInfo(
      builder, m_startTime, m_runNumber, instrumentName, m_streamOffset);

  auto messageFlatbuf = ISISDAE::CreateEventMessage(
      builder, ISISDAE::MessageTypes_RunInfo, messageRunInfo.Union());
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

flatbuffers::unique_ptr_t RunData::getRunBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunInfo = ISISDAE::CreateRunInfo(
      builder, m_startTime, m_runNumber, instrumentName, m_streamOffset);

  builder.Finish(messageRunInfo);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}

std::string RunData::runInfo() {
  std::stringstream ssRunInfo;
  ssRunInfo.imbue(std::locale("en_GB.utf-8"));
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
  ssRunInfo << ", Run offset: " << m_streamOffset;
  return ssRunInfo.str();
}
