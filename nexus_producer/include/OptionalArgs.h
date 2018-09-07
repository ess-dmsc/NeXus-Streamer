#pragma once

struct OptionalArgs {
  std::string filename;
  std::string detSpecFilename;
  std::string broker;
  std::string instrumentName = "test";
  std::string compression;
  bool slow = false;
  bool quietMode = false;
  bool singleRun = false;
  int32_t fakeEventsPerPulse = 0;
  float pulseRateInHz = 10;
};
