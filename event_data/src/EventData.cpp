#include "EventData.h"

EventData::EventData(const uint8_t *buf) { decodeMessage(buf); }

void EventData::decodeMessage(const uint8_t *buf) {
  auto frameData = ISISDAE::GetFramePart(buf);
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
}

flatbuffers::unique_ptr_t EventData::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto sEEvent = ISISDAE::CreateSEEvent(builder);
  std::vector<flatbuffers::Offset<ISISDAE::SEEvent>> sEEventsVector;
  sEEventsVector.push_back(sEEvent);
  auto messageSEEvents = builder.CreateVector(sEEventsVector);

  auto messageNEvents = ISISDAE::CreateNEvents(
      builder, builder.CreateVector(m_tof), builder.CreateVector(m_detId));

  auto messageFlatbuf = ISISDAE::CreateFramePart(
      builder, m_frameNumber, m_frameTime, ISISDAE::RunState_RUNNING,
      m_protonCharge, m_period, m_endOfFrame, m_endOfRun, messageNEvents,
      messageSEEvents);
  builder.Finish(messageFlatbuf);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
