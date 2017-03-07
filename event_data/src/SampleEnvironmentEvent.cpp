#include "SampleEnvironmentEvent.h"

BrightnESS::FlatBufs::f141_epics_nt::timeStamp_t
SampleEnvironmentEvent::getTimestamp() {
  // TODO convert from m_time
  using namespace BrightnESS::FlatBufs::f141_epics_nt;
  timeStamp_t timestamp(1, 1);
  return timestamp;
}
