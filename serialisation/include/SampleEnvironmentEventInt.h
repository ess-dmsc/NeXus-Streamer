#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventInt : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventInt(const std::string &name, float time, int32_t value,
                            uint64_t runStart)
      : SampleEnvironmentEvent(runStart, time, name), m_value(value) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  int32_t m_value;
};
