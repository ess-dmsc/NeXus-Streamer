#include "SampleEnvironmentEventDouble.h"

flatbuffers::Offset<ISISDAE::SEEvent>
SampleEnvironmentEventDouble::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = ISISDAE::CreateDoubleValue(builder, m_value);
  return ISISDAE::CreateSEEvent(builder, nameOffset, m_time,
                                ISISDAE::SEValue_DoubleValue, valueOffset.Union());
}
