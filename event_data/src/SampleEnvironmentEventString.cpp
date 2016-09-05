#include "SampleEnvironmentEventString.h"

flatbuffers::Offset<ISISStream::SEEvent>
SampleEnvironmentEventString::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto stringValueOffset = builder.CreateString(m_value);
  auto valueOffset = ISISStream::CreateStringValue(builder, stringValueOffset);
  return ISISStream::CreateSEEvent(builder, nameOffset, m_time,
                                   ISISStream::SEValue_StringValue, valueOffset.Union());
}
