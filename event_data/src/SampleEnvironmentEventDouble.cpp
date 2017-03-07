#include "SampleEnvironmentEventDouble.h"

flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
SampleEnvironmentEventDouble::getSEEvent(
    flatbuffers::FlatBufferBuilder &builder) {
  using namespace BrightnESS::FlatBufs::f141_epics_nt;
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateNTScalarDouble(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateEpicsPV(builder, nameOffset, PV_NTScalarDouble,
                       valueOffset.Union(), &timestamp);
}
