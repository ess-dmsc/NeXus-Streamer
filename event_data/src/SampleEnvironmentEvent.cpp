#include "SampleEnvironmentEvent.h"
#include <cmath>

uint64_t SampleEnvironmentEvent::getTimestamp() {
  // Get seconds since EPICS epoch (Jan 1 1990) from seconds since Unix epoch
  // (Jan 1 1970)
  // Don't be surprised by the round number, these timestamps ignore leap
  // seconds
  auto runStartSecondsPastEpicsEpoch =
      m_runStartSecondsPastUnixEpoch - 631152000L;
  int64_t secondsPastRunStart =
      runStartSecondsPastEpicsEpoch + std::lround(m_time);
  return static_cast<uint64_t>(secondsPastRunStart);
}

flatbuffers::unique_ptr_t
SampleEnvironmentEvent::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto sEEventMessage = getSEEvent(builder);
  builder.Finish(sEEventMessage);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
