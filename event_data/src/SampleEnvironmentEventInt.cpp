#include "SampleEnvironmentEventInt.h"

flatbuffers::Offset<ISISStream::SEEvent>
SampleEnvironmentEventInt::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISStream::CreateIntValue(builder, m_value);
  return ISISStream::CreateSEEvent(builder, nameOffset, m_time,
                                   ISISStream::SEValue_IntValue, valueOffset.Union());
}
