#pragma once

#include <type_traits>
#include <flatbuffers/flatbuffers.h>
#include "f142_logdata_generated.h"
#include "../../core/include/Message.h"

/*
 * To store SEEvent data in when reading the nexus file, and provide a method
 * for using it in the flatbuffers event message later
 */

class SampleEnvironmentEvent {
public:
  SampleEnvironmentEvent(uint64_t runStart, float eventTime, std::string name)
      : m_runStartNanosecondsPastUnixEpoch(runStart), m_time(eventTime),
        m_name(std::move(name)) {}
  virtual ~SampleEnvironmentEvent() = default;

  virtual flatbuffers::Offset<LogData>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) = 0;
  std::string getName() { return m_name; }
  float getTime() { return m_time; }
  uint64_t getTimestamp();
  Streamer::Message getBuffer();

protected:
  uint64_t m_runStartNanosecondsPastUnixEpoch;
  float m_time;
  std::string m_name;
};
