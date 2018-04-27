#pragma once

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventDouble : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventDouble(const std::string &name, float time,
                               double value, uint64_t runStart)
      : m_value(value), SampleEnvironmentEvent(runStart, time, name) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  double m_value;
};
