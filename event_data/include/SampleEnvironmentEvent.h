#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H

#include "f141_epics_nt_generated.h"
#include <type_traits>

/*
 * To store SEEvent data in when reading the nexus file, and provide a method
 * for using it in the flatbuffers event message later
 */

class SampleEnvironmentEvent {
public:
  SampleEnvironmentEvent(int64_t runStart, float eventTime,
                         const std::string &name)
      : m_runStartSecondsPastUnixEpoch(runStart), m_time(eventTime),
        m_name(name) {}
  virtual ~SampleEnvironmentEvent() {}

  virtual flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) = 0;
  std::string getName() { return m_name; }
  float getTime() { return m_time; }
  BrightnESS::FlatBufs::f141_epics_nt::timeStamp_t getTimestamp();
  flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer);

protected:
  std::string m_name;
  int64_t m_runStartSecondsPastUnixEpoch;
  float m_time;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
