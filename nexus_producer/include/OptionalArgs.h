#pragma once

#include <string>

struct OptionalArgs {
  std::pair<int32_t, int32_t> minMaxDetectorNums = {0, 0};
  std::string filename;
  std::string detSpecFilename;
  std::string broker;
  std::string instrumentName = "test";
  std::string compression;
  std::string jsonDescription;
  bool slow = false;
  bool quietMode = false;
  bool singleRun = false;
  int32_t fakeEventsPerPulse = 0;
  uint32_t histogramUpdatePeriodMs = 0;
};
