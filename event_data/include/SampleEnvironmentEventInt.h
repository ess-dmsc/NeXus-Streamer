#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventInt : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventInt(const std::string &name, float time, int32_t value,
                            int64_t runStart)
      : m_value(value), SampleEnvironmentEvent(runStart, time, name) {}

  flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  int32_t m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H
