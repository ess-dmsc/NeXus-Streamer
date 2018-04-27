#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventLong : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventLong(const std::string &name, float time, int64_t value,
                             uint64_t runStart)
      : m_value(value), SampleEnvironmentEvent(runStart, time, name) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  int64_t m_value;
};
