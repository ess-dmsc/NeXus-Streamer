#pragma once

#include <ctime>
#include <fmt/format.h>
#include <iomanip>

#include "../../core/include/Message.h"
#include "UUID.h"

struct RunDataPOD {
  uint64_t startTime{0};
  uint64_t stopTime{0};
  std::string runID;
  std::string instrumentName;
  int32_t numberOfPeriods{1};
  std::string nexusStructure;
  std::string jobID{generate_uuid(18)};
  std::string serviceID; // Optional for file writer (we won't populate it)

  void setStartTimeFromString(const std::string &inputTime);
  void setStopTimeFromString(const std::string &inputTime);
  void setStartTimeInSeconds(time_t inputTime);
};

template <> struct fmt::formatter<RunDataPOD> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const RunDataPOD &runData, FormatContext &ctx) {
    const auto sTime = static_cast<time_t>(runData.startTime / 1000000000);
    return format_to(ctx.out(),
                     "Run ID: {}, Instrument name: {}, Start time: {}",
                     runData.runID, runData.instrumentName,
                     std::put_time(std::gmtime(&sTime), "%Y-%m-%dT%H:%M:%S"));
  }
};

Streamer::Message serialiseRunStartMessage(RunDataPOD &runData);
Streamer::Message serialiseRunStopMessage(const RunDataPOD &runData);
RunDataPOD deserialiseRunStartMessage(const uint8_t *buffer);
RunDataPOD deserialiseRunStopMessage(const uint8_t *buffer);
