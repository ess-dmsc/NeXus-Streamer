#include "SampleEnvironmentEventInt.h"

flatbuffers::Offset<LogData>
SampleEnvironmentEventInt::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateInt(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value::Int, valueOffset.Union(),
                       timestamp);
}
