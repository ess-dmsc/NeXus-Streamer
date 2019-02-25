#include "SampleEnvironmentEventLong.h"
#include <cmath>

flatbuffers::Offset<LogData> SampleEnvironmentEventLong::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateLong(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value::Long, valueOffset.Union(),
                       timestamp);
}
