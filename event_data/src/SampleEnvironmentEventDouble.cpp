#include "SampleEnvironmentEventDouble.h"

flatbuffers::Offset<LogData> SampleEnvironmentEventDouble::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateDouble(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateLogData(builder, nameOffset, Value_Double, valueOffset.Union(),
                       timestamp);
}
