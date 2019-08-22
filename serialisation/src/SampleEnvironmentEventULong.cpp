#include "SampleEnvironmentEventULong.h"

flatbuffers::Offset<LogData> SampleEnvironmentEventULong::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateULong(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value::ULong, valueOffset.Union(),
                       timestamp);
}
