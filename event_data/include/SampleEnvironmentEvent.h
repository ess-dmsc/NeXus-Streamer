#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H

#include "f142_logdata_generated.h"
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

  virtual flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) = 0;
  std::string getName() { return m_name; }
  float getTime() { return m_time; }
  uint64_t getTimestamp();
  flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer);
  size_t getBufferSize() { return m_bufferSize; }

protected:
  size_t m_bufferSize;
  std::string m_name;
  int64_t m_runStartSecondsPastUnixEpoch;
  float m_time;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
