#pragma once

#include <cstdint>
#include <vector>

namespace Streamer {
class Message;
}

Streamer::Message createHistogramMessage(const std::vector<int32_t> &counts,
                                         const std::vector<size_t> &countsShape,
                                         const std::vector<float> &timeOfFlight,
                                         uint64_t timestampUnix);

void deserialiseHistogramMessage(const Streamer::Message &message,
                                 std::vector<int32_t> &counts,
                                 std::vector<size_t> &countsShape,
                                 std::vector<float> &timeOfFlight,
                                 uint64_t &timestampUnix);
