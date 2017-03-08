#include "SampleEnvironmentEventInt.h"

flatbuffers::Offset<BrightnESS::FlatBufs::f141_epics_nt::EpicsPV>
SampleEnvironmentEventInt::getSEEvent(flatbuffers::FlatBufferBuilder &builder) {
  using namespace BrightnESS::FlatBufs::f141_epics_nt;
  auto nameOffset = builder.CreateString(m_name);
  auto valueOffset = CreateNTScalarInt(builder, m_value);
  auto timestamp = getTimestamp();
  return CreateEpicsPV(builder, nameOffset, PV_NTScalarInt, valueOffset.Union(),
                       &timestamp);
}
