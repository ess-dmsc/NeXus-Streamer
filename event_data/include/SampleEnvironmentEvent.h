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
  virtual ~SampleEnvironmentEvent() {}
  virtual flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
  getSEEvent(flatbuffers::FlatBufferBuilder &builder) = 0;

  BrightnESS::FlatBufs::f141_epics_nt::timeStamp_t getTimestamp();
  virtual std::string getName() = 0;
  virtual float getTime() = 0;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_SAMPLEENVIRONMENTEVENT_H
