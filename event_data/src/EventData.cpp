#include "EventData.h"
#include <iostream>

bool EventData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISDAE::GetEventMessage(buf);
  if (messageData->message_type() == ISISDAE::MessageTypes_FramePart) {
    auto frameData = static_cast<const ISISDAE::FramePart*>(messageData->message());
    auto eventData = frameData->n_events();
    auto detIdFBVector = eventData->spec();
    auto tofFBVector = eventData->tof();

    auto numberOfEvents = detIdFBVector->size();
    m_detId.resize(static_cast<size_t>(numberOfEvents));
    m_tof.resize(static_cast<size_t>(numberOfEvents));
    std::copy(detIdFBVector->begin(), detIdFBVector->end(), m_detId.begin());
    std::copy(tofFBVector->begin(), tofFBVector->end(), m_tof.begin());

    setFrameNumber(frameData->frame_number());
    setTotalCounts(numberOfEvents);
    setEndOfFrame(frameData->end_of_frame());
    setEndOfRun(frameData->end_of_run());
    setProtonCharge(frameData->proton_charge());
    setFrameTime(frameData->frame_time());
    setPeriod(frameData->period());
    return true;
  }
  return false; // this is not an EventData message
}

flatbuffers::unique_ptr_t EventData::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto sEEvent = ISISDAE::CreateSEEvent(builder);
  std::vector<flatbuffers::Offset<ISISDAE::SEEvent>> sEEventsVector;
  sEEventsVector.push_back(sEEvent);
  auto messageSEEvents = builder.CreateVector(sEEventsVector);

  auto messageNEvents = ISISDAE::CreateNEvents(
      builder, builder.CreateVector(m_tof), builder.CreateVector(m_detId));

  auto messageFramePart = ISISDAE::CreateFramePart(
      builder, m_frameNumber, m_frameTime, ISISDAE::RunState_RUNNING,
      m_protonCharge, m_period, m_endOfFrame, m_endOfRun, messageNEvents,
      messageSEEvents);

  auto messageFlatbuf = ISISDAE::CreateEventMessage(builder, ISISDAE::MessageTypes_FramePart, messageFramePart.Union());
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
