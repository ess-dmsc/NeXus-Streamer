#include "EventData.h"
#include <iostream>

bool EventData::decodeMessage(const uint8_t *buf) {
  auto messageData = GetEventMessage(buf);
  if (messageData->facility_specific_data_type() == FacilityData_ISISData) {
    auto detIdFBVector = messageData->detector_id();
    auto tofFBVector = messageData->time_of_flight();
    auto numberOfEvents = detIdFBVector->size();
    m_detId.resize(static_cast<size_t>(numberOfEvents));
    m_tof.resize(static_cast<size_t>(numberOfEvents));
    std::copy(detIdFBVector->begin(), detIdFBVector->end(), m_detId.begin());
    std::copy(tofFBVector->begin(), tofFBVector->end(), m_tof.begin());
    setTotalCounts(numberOfEvents);
    setFrameTime(messageData->pulse_time());

    auto isisData =
        static_cast<const ISISData *>(messageData->facility_specific_data());
    setPeriod(isisData->period_number());
    setProtonCharge(isisData->proton_charge());
    return true;
  }
  return false; // this is not an ISIS EventData message
}

flatbuffers::unique_ptr_t EventData::getBufferPointer(std::string &buffer,
                                                      uint64_t messageID) {
  flatbuffers::FlatBufferBuilder builder;

  auto isisDataMessage =
    CreateISISData(builder, m_period, RunState_RUNNING, m_protonCharge);

  std::uint8_t *tempTofBuffer;
  std::uint8_t *tempDetIdBuffer;
  auto eventMessage = CreateEventMessage(
    builder, builder.CreateString("isis_nexus_streamer_for_mantid"),
    messageID, m_frameTime,
    builder.CreateUninitializedVector(m_tof.size(), sizeof(uint32_t),
                                      &tempTofBuffer),
    builder.CreateUninitializedVector(m_detId.size(), sizeof(uint32_t),
                                      &tempDetIdBuffer),
    FacilityData_ISISData, isisDataMessage.Union());
  std::memcpy(tempTofBuffer, m_tof.data(), m_tof.size() * sizeof(uint32_t));
  std::memcpy(tempDetIdBuffer, m_detId.data(),
              m_detId.size() * sizeof(uint32_t));

  FinishEventMessageBuffer(builder, eventMessage);

  auto bufferpointer =
    reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
