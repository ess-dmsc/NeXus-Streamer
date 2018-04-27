#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventInt : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventInt(const std::string &name, float time, int32_t value,
                            uint64_t runStart)
      : m_value(value), SampleEnvironmentEvent(runStart, time, name) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  int32_t m_value;
};
