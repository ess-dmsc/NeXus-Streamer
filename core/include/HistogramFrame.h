#pragma once

#include <cstdint>
#include <vector>

struct HistogramFrame {
  HistogramFrame(std::vector<int32_t> detectorCounts,
                 std::vector<size_t> countsShape,
                 std::vector<float> tofBinEdges)
      : counts(std::move(detectorCounts)), countsShape(std::move(countsShape)),
        timeOfFlight(std::move(tofBinEdges)) {}
  std::vector<int32_t> counts;
  std::vector<size_t> countsShape;
  std::vector<float> timeOfFlight;
};
