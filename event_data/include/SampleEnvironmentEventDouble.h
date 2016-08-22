#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventDouble : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventDouble() {}
  SampleEnvironmentEventDouble(const std::string &name, float time,
                               double value)
      : m_name(name), m_time(time), m_value(value) {}

  flatbuffers::Offset<ISISDAE::SEEvent>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

  std::string getName() override { return m_name; }
  float getTime() override { return m_time; }

private:
  std::string m_name;
  float m_time;
  double m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTDOUBLE_H
