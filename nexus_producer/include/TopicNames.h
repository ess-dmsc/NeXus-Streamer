#pragma once
#include <string>

struct TopicNames {
  explicit TopicNames(const std::string &instrumentName)
      : sampleEnv(instrumentName + "_sampleEnv"),
        event(instrumentName + "_events"),
        histogram(instrumentName + "_histograms"),
        runInfo(instrumentName + "_runInfo"),
        detSpecMap(instrumentName + "_detSpecMap"){};
  const std::string sampleEnv;
  const std::string event;
  const std::string histogram;
  const std::string runInfo;
  const std::string detSpecMap;
};
