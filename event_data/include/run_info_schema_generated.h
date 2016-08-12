// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_RUNINFOSCHEMA_ISISDAE_H_
#define FLATBUFFERS_GENERATED_RUNINFOSCHEMA_ISISDAE_H_

#include "flatbuffers/flatbuffers.h"


namespace ISISDAE {

struct RunInfo;

struct RunInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_START_TIME = 4,
    VT_RUN_NUMBER = 6,
    VT_INST_NAME = 8,
    VT_STREAM_OFFSET = 10
  };
  uint64_t start_time() const { return GetField<uint64_t>(VT_START_TIME, 0); }
  int32_t run_number() const { return GetField<int32_t>(VT_RUN_NUMBER, 0); }
  const flatbuffers::String *inst_name() const { return GetPointer<const flatbuffers::String *>(VT_INST_NAME); }
  int64_t stream_offset() const { return GetField<int64_t>(VT_STREAM_OFFSET, 0); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_START_TIME) &&
           VerifyField<int32_t>(verifier, VT_RUN_NUMBER) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_INST_NAME) &&
           verifier.Verify(inst_name()) &&
           VerifyField<int64_t>(verifier, VT_STREAM_OFFSET) &&
           verifier.EndTable();
  }
};

struct RunInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_start_time(uint64_t start_time) { fbb_.AddElement<uint64_t>(RunInfo::VT_START_TIME, start_time, 0); }
  void add_run_number(int32_t run_number) { fbb_.AddElement<int32_t>(RunInfo::VT_RUN_NUMBER, run_number, 0); }
  void add_inst_name(flatbuffers::Offset<flatbuffers::String> inst_name) { fbb_.AddOffset(RunInfo::VT_INST_NAME, inst_name); }
  void add_stream_offset(int64_t stream_offset) { fbb_.AddElement<int64_t>(RunInfo::VT_STREAM_OFFSET, stream_offset, 0); }
  RunInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  RunInfoBuilder &operator=(const RunInfoBuilder &);
  flatbuffers::Offset<RunInfo> Finish() {
    auto o = flatbuffers::Offset<RunInfo>(fbb_.EndTable(start_, 4));
    return o;
  }
};

inline flatbuffers::Offset<RunInfo> CreateRunInfo(flatbuffers::FlatBufferBuilder &_fbb,
   uint64_t start_time = 0,
   int32_t run_number = 0,
   flatbuffers::Offset<flatbuffers::String> inst_name = 0,
   int64_t stream_offset = 0) {
  RunInfoBuilder builder_(_fbb);
  builder_.add_stream_offset(stream_offset);
  builder_.add_start_time(start_time);
  builder_.add_inst_name(inst_name);
  builder_.add_run_number(run_number);
  return builder_.Finish();
}

inline const ISISDAE::RunInfo *GetRunInfo(const void *buf) { return flatbuffers::GetRoot<ISISDAE::RunInfo>(buf); }

inline bool VerifyRunInfoBuffer(flatbuffers::Verifier &verifier) { return verifier.VerifyBuffer<ISISDAE::RunInfo>(); }

inline void FinishRunInfoBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<ISISDAE::RunInfo> root) { fbb.Finish(root); }

}  // namespace ISISDAE

#endif  // FLATBUFFERS_GENERATED_RUNINFOSCHEMA_ISISDAE_H_
