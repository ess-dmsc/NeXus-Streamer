#pragma once

#include <string>

class EventPublisher {
public:
  virtual ~EventPublisher() = default;
  virtual void setUp(const std::string &broker,
                     const std::string &instrumentName) = 0;
  virtual void sendEventMessage(char *buf, size_t messageSize) = 0;
  virtual void sendRunMessage(char *buf, size_t messageSize) = 0;
  virtual void sendDetSpecMessage(char *buf, size_t messageSize) = 0;
  virtual void sendSampleEnvMessage(char *buf, size_t messageSize) = 0;
  virtual void flushSendQueue() = 0;
  virtual int64_t getCurrentOffset() = 0;
};
