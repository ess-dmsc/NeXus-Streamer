#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H

#include "event_schema_generated.h"
#include <type_traits>

/*
 * To store SEEvent data in when reading the nexus file, and provide a method
 * for using it in the flatbuffers event message later
 */

class SampleEnvironmentEvent {
public:
  virtual ~SampleEnvironmentEvent() {}
  virtual flatbuffers::Offset<ISISDAE::SEEvent>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) = 0;

  virtual std::string getName() = 0;
  virtual float getTime() = 0;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
