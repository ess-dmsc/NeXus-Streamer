#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventUInt : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventUInt(const std::string &name, float time,
                             uint32_t value, uint64_t runStart)
      : SampleEnvironmentEvent(runStart, time, name), m_value(value) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  uint32_t m_value;
};
