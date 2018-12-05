#include "SampleEnvironmentEventULong.h"
#include <cmath>

flatbuffers::Offset<LogData> SampleEnvironmentEventULong::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateULong(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value_ULong, valueOffset.Union(),
                       timestamp);
}
