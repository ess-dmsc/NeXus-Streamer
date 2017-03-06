#include "EventData.h"
#include <iostream>

uint64_t getMessageID(const std::string &rawbuf) {
  auto buf = reinterpret_cast<const uint8_t *>(rawbuf.c_str());
  auto messageData = ISISStream::GetEventMessage(buf);
  return messageData->id();
}

bool EventData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISStream::GetEventMessage(buf);
  if (messageData->message_type() == ISISStream::MessageTypes_FramePart) {
    auto frameData =
        static_cast<const ISISStream::FramePart *>(messageData->message());
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

flatbuffers::unique_ptr_t EventData::getBufferPointer(std::string &buffer,
                                                      uint64_t messageID) {
  flatbuffers::FlatBufferBuilder builder;

  auto messageNEvents = ISISStream::CreateNEvents(
      builder, builder.CreateVector(m_tof), builder.CreateVector(m_detId));

  auto messageFramePart = ISISStream::CreateFramePart(
      builder, m_frameNumber, m_frameTime, ISISStream::RunState_RUNNING,
      m_protonCharge, m_period, m_endOfFrame, m_endOfRun, messageNEvents);

  auto messageFlatbuf =
      ISISStream::CreateEventMessage(builder, ISISStream::MessageTypes_FramePart,
                                  messageFramePart.Union(), messageID);
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
