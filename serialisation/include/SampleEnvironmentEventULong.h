#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventULong : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventULong(const std::string &name, float time,
                              uint64_t value, uint64_t runStart)
      : SampleEnvironmentEvent(runStart, time, name), m_value(value) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  uint64_t m_value;
};
