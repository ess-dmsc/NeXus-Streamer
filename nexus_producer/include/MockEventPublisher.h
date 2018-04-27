#pragma once

#include "EventPublisher.h"
#include <gmock/gmock.h>

class MockEventPublisher : public EventPublisher {
public:
  MOCK_METHOD2(setUp, void(const std::string &broker,
                           const std::string &instrumentName));
  MOCK_METHOD2(sendRunMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD2(sendDetSpecMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD2(sendEventMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD2(sendSampleEnvMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD0(getCurrentOffset, int64_t());
  MOCK_METHOD0(flushSendQueue, void());
};
