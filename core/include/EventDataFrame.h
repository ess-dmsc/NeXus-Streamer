#pragma once

struct EventDataFrame {
  EventDataFrame(std::vector<uint32_t> detIDs, std::vector<uint32_t> tofs)
      : detectorIDs(std::move(detIDs)), timeOfFlights(std::move(tofs)) {}
  std::vector<uint32_t> detectorIDs;
  std::vector<uint32_t> timeOfFlights;
};
