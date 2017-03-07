#include "SampleEnvironmentEvent.h"

BrightnESS::FlatBufs::f141_epics_nt::timeStamp_t
SampleEnvironmentEvent::getTimestamp() {
  using namespace BrightnESS::FlatBufs::f141_epics_nt;
  // Get seconds since EPICS epoch (Jan 1 1990) from seconds since Unix epoch
  // (Jan 1 1970)
  // Don't be surprised by the round number, these timestamps ignore leap
  // seconds
  auto runStartSecondsPastEpicsEpoch =
      m_runStartSecondsPastUnixEpoch - 631152000;
  int64_t secondsPastRunStart =
      runStartSecondsPastEpicsEpoch + std::lround(m_time);
  // use 0 nanoseconds as calculated from start of run which is only known
  // to seconds anyway
  timeStamp_t timestamp(static_cast<uint64_t>(secondsPastRunStart), 0);
  return timestamp;
}