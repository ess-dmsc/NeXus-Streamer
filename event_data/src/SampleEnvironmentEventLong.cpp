#include "SampleEnvironmentEventLong.h"

flatbuffers::Offset<ISISDAE::SEEvent>
SampleEnvironmentEventLong::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISDAE::CreateLongValue(builder, m_value);
  return ISISDAE::CreateSEEvent(builder, nameOffset, m_time,
                                ISISDAE::SEValue_LongValue, valueOffset.Union());
}
