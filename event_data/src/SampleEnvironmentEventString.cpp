#include "SampleEnvironmentEventString.h"

flatbuffers::Offset<ISISDAE::SEEvent>
SampleEnvironmentEventString::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto stringValueOffset = builder.CreateString(m_value);
  auto valueOffset = ISISDAE::CreateStringValue(builder, stringValueOffset);
  return ISISDAE::CreateSEEvent(builder, nameOffset, m_time,
                                ISISDAE::SEValue_StringValue, valueOffset.Union());
}
