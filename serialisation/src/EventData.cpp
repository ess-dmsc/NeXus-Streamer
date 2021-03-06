#include <ev42_events_generated.h>
#include <is84_isis_events_generated.h>

#include "../../core/include/Message.h"
#include "EventData.h"

bool EventData::decodeMessage(const uint8_t *buf) {
  auto messageData = GetEventMessage(buf);
  auto detIdFBVector = messageData->detector_id();
  auto tofFBVector = messageData->time_of_flight();
  auto numberOfEvents = detIdFBVector->size();
  m_detId.resize(static_cast<size_t>(numberOfEvents));
  m_tof.resize(static_cast<size_t>(numberOfEvents));
  std::copy(detIdFBVector->begin(), detIdFBVector->end(), m_detId.begin());
  std::copy(tofFBVector->begin(), tofFBVector->end(), m_tof.begin());
  setTotalCounts(numberOfEvents);
  setFrameTime(messageData->pulse_time());

  if (messageData->facility_specific_data_type() == FacilityData::ISISData) {
    auto isisData =
        static_cast<const ISISData *>(messageData->facility_specific_data());
    setPeriod(isisData->period_number());
    setProtonCharge(isisData->proton_charge());
    return true; // this is an ISIS facility event message
  }

  return false; // this is not an ISIS facility event message
}

Streamer::Message EventData::getBuffer(uint64_t messageID) {
  flatbuffers::FlatBufferBuilder builder;

  auto isisDataMessage =
      CreateISISData(builder, m_period, RunState::RUNNING, m_protonCharge);

  auto detIDData = builder.CreateVector(m_detId);
  auto tofData = builder.CreateVector(m_tof);

  auto sourceStr = builder.CreateString("NeXus-Streamer");

  EventMessageBuilder eventMessageBuilder(builder);
  eventMessageBuilder.add_source_name(sourceStr);
  eventMessageBuilder.add_message_id(messageID);
  eventMessageBuilder.add_pulse_time(m_frameTime);
  eventMessageBuilder.add_time_of_flight(tofData);
  eventMessageBuilder.add_detector_id(detIDData);

  if (m_period < std::numeric_limits<uint32_t>::max() && m_protonCharge > 0) {
    eventMessageBuilder.add_facility_specific_data_type(FacilityData::ISISData);
    eventMessageBuilder.add_facility_specific_data(isisDataMessage.Union());
  } else {
    eventMessageBuilder.add_facility_specific_data_type(FacilityData::NONE);
  }

  auto eventMessage = eventMessageBuilder.Finish();
  FinishEventMessageBuffer(builder, eventMessage);

  return Streamer::Message(builder.Release());
}
