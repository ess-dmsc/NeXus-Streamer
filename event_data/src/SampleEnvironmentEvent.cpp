#include "SampleEnvironmentEvent.h"
#include <cmath>

uint64_t SampleEnvironmentEvent::getTimestamp() {
  auto nanosecondsPastRunStart = static_cast<uint64_t>(m_time * 1e9);
  return nanosecondsPastRunStart + m_runStartNanosecondsPastUnixEpoch;
}

Streamer::Message SampleEnvironmentEvent::getBuffer() {
  flatbuffers::FlatBufferBuilder builder;

  auto sEEventMessage = getSEEvent(builder);
  FinishLogDataBuffer(builder, sEEventMessage);

  return Streamer::Message(builder.Release());
}
