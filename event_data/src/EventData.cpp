#include "EventData.h"
#include <SampleEnvironmentEventDouble.h>
#include <SampleEnvironmentEventInt.h>
#include <SampleEnvironmentEventLong.h>
#include <SampleEnvironmentEventString.h>
#include <iostream>

uint64_t getMessageID(const std::string &rawbuf) {
  auto buf = reinterpret_cast<const uint8_t *>(rawbuf.c_str());
  auto messageData = ISISStream::GetEventMessage(buf);
  return messageData->id();
}

void EventData::decodeSampleEnvironmentEvents(
    const flatbuffers::Vector<flatbuffers::Offset<ISISStream::SEEvent>>
        *sEEventVector) {
  for (flatbuffers::uoffset_t i = 0; i < sEEventVector->Length(); i++) {
    auto event = sEEventVector->Get(i);
    if (event->value_type() == ISISStream::SEValue_IntValue) {
      auto value = static_cast<const ISISStream::IntValue *>(event->value());
      auto sEEvent = std::make_shared<SampleEnvironmentEventInt>(
          event->name()->str(), event->time_offset(), value->value());
      addSEEvent(sEEvent);
    } else if (event->value_type() == ISISStream::SEValue_LongValue) {
      auto value = static_cast<const ISISStream::LongValue *>(event->value());
      auto sEEvent = std::make_shared<SampleEnvironmentEventLong>(
          event->name()->str(), event->time_offset(), value->value());
      addSEEvent(sEEvent);
    } else if (event->value_type() == ISISStream::SEValue_DoubleValue) {
      auto value = static_cast<const ISISStream::DoubleValue *>(event->value());
      auto sEEvent = std::make_shared<SampleEnvironmentEventDouble>(
          event->name()->str(), event->time_offset(), value->value());
      addSEEvent(sEEvent);
    } else if (event->value_type() == ISISStream::SEValue_StringValue) {
      auto value = static_cast<const ISISStream::StringValue *>(event->value());
      auto sEEvent = std::make_shared<SampleEnvironmentEventString>(
          event->name()->str(), event->time_offset(), value->value()->str());
      addSEEvent(sEEvent);
    } else {
      std::cout << "SEValue was not of recognised type" << std::endl;
    }
  }
}

bool EventData::decodeMessage(const uint8_t *buf) {
  auto messageData = ISISStream::GetEventMessage(buf);
  if (messageData->message_type() == ISISStream::MessageTypes_FramePart) {
    auto frameData =
        static_cast<const ISISStream::FramePart *>(messageData->message());
    auto eventData = frameData->n_events();
    auto detIdFBVector = eventData->spec();
    auto tofFBVector = eventData->tof();
    auto sEEventVector = frameData->se_events();

    decodeSampleEnvironmentEvents(sEEventVector);

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

  std::vector<flatbuffers::Offset<ISISStream::SEEvent>> sEEventsVector;
  for (auto sEEvent : m_sampleEnvironmentEvents) {
    auto sEEventOffset = sEEvent->getSEEvent(builder);
    sEEventsVector.push_back(sEEventOffset);
  }
  auto messageSEEvents = builder.CreateVector(sEEventsVector);

  auto messageNEvents = ISISStream::CreateNEvents(
      builder, builder.CreateVector(m_tof), builder.CreateVector(m_detId));

  auto messageFramePart = ISISStream::CreateFramePart(
      builder, m_frameNumber, m_frameTime, ISISStream::RunState_RUNNING,
      m_protonCharge, m_period, m_endOfFrame, m_endOfRun, messageNEvents,
      messageSEEvents);

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
