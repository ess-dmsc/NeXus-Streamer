#include "SampleEnvironmentEventLong.h"

flatbuffers::Offset<ISISStream::SEEvent>
SampleEnvironmentEventLong::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISStream::CreateLongValue(builder, m_value);
  return ISISStream::CreateSEEvent(builder, nameOffset, m_time,
                                   ISISStream::SEValue_LongValue, valueOffset.Union());
}
