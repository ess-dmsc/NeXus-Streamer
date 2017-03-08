#include "SampleEnvironmentEventLong.h"
#include <cmath>

flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
SampleEnvironmentEventLong::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  using namespace BrightnESS::FlatBufs::f141_epics_nt;
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateNTScalarLong(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateEpicsPV(builder, nameOffset, PV_NTScalarLong,
                       valueOffset.Union(), &timestamp);
}
