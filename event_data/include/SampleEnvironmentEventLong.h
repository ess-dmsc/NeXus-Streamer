#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventLong : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventLong() {}
  SampleEnvironmentEventLong(const std::string &name, float time, int64_t value)
      : m_name(name), m_time(time), m_value(value) {}

  flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;
  std::string getName() override { return m_name; }
  float getTime() override { return m_time; }

private:
  std::string m_name;
  float m_time;
  int64_t m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTLONG_H
