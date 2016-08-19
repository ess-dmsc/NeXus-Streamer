#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventLong : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventLong() {}
  SampleEnvironmentEventLong(const std::string &name, float time, int64_t value)
      : m_name(name), m_time(time), m_value(value) {}

  flatbuffers::Offset<ISISDAE::SEEvent>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  std::string m_name;
  float m_time;
  int64_t m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H
