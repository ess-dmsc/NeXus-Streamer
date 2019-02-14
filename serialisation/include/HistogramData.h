#pragma once

#include <cstdint>
#include <vector>

namespace Streamer {
class Message;
}
struct HistogramFrame;

Streamer::Message createHistogramMessage(const HistogramFrame &histogram,
                                         uint64_t timestampUnix);

HistogramFrame deserialiseHistogramMessage(Streamer::Message &message,
                                           uint64_t &timestampUnix);
