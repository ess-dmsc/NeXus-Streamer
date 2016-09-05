#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTSTRING_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTSTRING_H

#include "SampleEnvironmentEvent.h"

class SampleEnvironmentEventString : public SampleEnvironmentEvent {
public:
  SampleEnvironmentEventString() {}
  SampleEnvironmentEventString(const std::string &name, float time,
                               const std::string &value)
      : m_name(name), m_time(time), m_value(value) {}

  flatbuffers::Offset<ISISStream::SEEvent>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) override;

  std::string getName() override { return m_name; }
  float getTime() override { return m_time; }

private:
  std::string m_name;
  float m_time;
  std::string m_value;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENTSTRING_H
