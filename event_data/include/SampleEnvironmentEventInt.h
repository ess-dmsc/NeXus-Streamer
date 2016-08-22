#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventInt : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventInt() {}
  SampleEnvironmentEventInt(const std::string &name, float time, int32_t value)
      : m_name(name), m_time(time), m_value(value) {}

  flatbuffers::Offset<ISISDAE::SEEvent>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

  std::string getName() override { return m_name; }
  float getTime() override { return m_time; }

private:
  std::string m_name;
  float m_time;
  int32_t m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTINT_H
