#pragma once

#include <vector>
#include <cstdint>

namespace Streamer {
class Message;
}

Streamer::Message createHistogramMessage(const std::vector<int32_t> &counts, const std::vector<float> &timeOfFlight);
