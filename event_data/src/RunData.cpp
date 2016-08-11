#include "RunData.h"

#include <sstream>
#include <ctime>
#include <iostream>
#include <iomanip>

void RunData::setStartTime(const std::string &inputTime) {
  std::istringstream ss(inputTime);
  std::tm tmb = {};
  ss >> std::get_time(&tmb, "%Y-%m-%dT%H:%M:%S");
  m_startTime = static_cast<uint64_t>(std::mktime(&tmb));
}

bool RunData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISDAE::GetEventMessage(buf);
  if (messageData->message_type() == ISISDAE::MessageTypes_RunInfo) {
    auto runData = static_cast<const ISISDAE::RunInfo*>(messageData->message());

    setStartTime(runData->start_time());
    setInstrumentName(runData->inst_name()->str());
    setRunNumber(runData->run_number());

    return true;
  }
  return false; // this is not a RunData message
}

flatbuffers::unique_ptr_t RunData::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto instrumentName = builder.CreateString(m_instrumentName);
  auto messageRunInfo = ISISDAE::CreateRunInfo(
      builder, m_startTime, m_runNumber, instrumentName);

  auto messageFlatbuf = ISISDAE::CreateEventMessage(builder, ISISDAE::MessageTypes_RunInfo, messageRunInfo.Union());
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
