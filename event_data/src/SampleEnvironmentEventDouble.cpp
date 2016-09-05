#include "SampleEnvironmentEventDouble.h"

flatbuffers::Offset<ISISStream::SEEvent> SampleEnvironmentEventDouble::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISStream::CreateDoubleValue(builder, m_value);
  return ISISStream::CreateSEEvent(builder, nameOffset, m_time,
                                   ISISStream::SEValue_DoubleValue,
                                valueOffset.Union());
}
