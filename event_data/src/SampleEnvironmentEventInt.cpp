#include "SampleEnvironmentEventInt.h"

flatbuffers::Offset<ISISDAE::SEEvent>
SampleEnvironmentEventInt::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISDAE::CreateIntValue(builder, m_value);
  return ISISDAE::CreateSEEvent(builder, nameOffset, m_time,
                                ISISDAE::SEValue_IntValue, valueOffset.Union());
}
