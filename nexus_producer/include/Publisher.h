#pragma once

#include <string>

namespace Streamer {
class Message;
}

class Publisher {
public:
  virtual ~Publisher() = default;
  virtual void setUp(const std::string &broker,
                     const std::string &instrumentName) = 0;
  virtual void sendEventMessage(Streamer::Message &message) = 0;
  virtual void sendRunMessage(Streamer::Message &message) = 0;
  virtual void sendDetSpecMessage(Streamer::Message &message) = 0;
  virtual void sendSampleEnvMessage(Streamer::Message &message) = 0;
  virtual void sendHistogramMessage(Streamer::Message &message) = 0;
  virtual void flushSendQueue() = 0;
  virtual int64_t getCurrentOffset() = 0;
};
