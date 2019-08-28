#include "SampleEnvironmentEventUInt.h"

flatbuffers::Offset<LogData> SampleEnvironmentEventUInt::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateUInt(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value::UInt, valueOffset.Union(),
                       timestamp);
}
