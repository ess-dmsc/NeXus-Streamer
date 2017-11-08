#include "SampleEnvironmentEvent.h"
#include <cmath>

uint64_t SampleEnvironmentEvent::getTimestamp() {
  auto nanosecondsPastRunStart = static_cast<uint64_t>(m_time * 1e9);
  return nanosecondsPastRunStart + m_runStartNanosecondsPastUnixEpoch;
}

flatbuffers::unique_ptr_t
SampleEnvironmentEvent::getBufferPointer(std::string &buffer) {
  flatbuffers::FlatBufferBuilder builder;

  auto sEEventMessage = getSEEvent(builder);
  FinishLogDataBuffer(builder, sEEventMessage);

  auto bufferpointer =
      reinterpret_cast<const char *>(builder.GetBufferPointer());
  buffer.assign(bufferpointer, bufferpointer + builder.GetSize());

  m_bufferSize = builder.GetSize();

  return builder.ReleaseBufferPointer();
}
