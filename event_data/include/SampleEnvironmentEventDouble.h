#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventDouble : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventDouble(const std::string &name, float time,
                               double value, int64_t runStart)
      : m_value(value), SampleEnvironmentEvent(runStart, time, name) {}

  flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

private:
  double m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H
