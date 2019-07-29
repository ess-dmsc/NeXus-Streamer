#pragma once

#include <gmock/gmock.h>

#include "Publisher.h"

class MockPublisher : public Publisher {
public:
  MOCK_METHOD2(setUp, void(const std::string &broker,
                           const std::string &instrumentName));
  MOCK_METHOD1(sendRunMessage, void(Streamer::Message &message));
  MOCK_METHOD1(sendDetSpecMessage, void(Streamer::Message &message));
  MOCK_METHOD1(sendEventMessage, void(Streamer::Message &message));
  MOCK_METHOD1(sendSampleEnvMessage, void(Streamer::Message &message));
  MOCK_METHOD1(sendHistogramMessage, void(Streamer::Message &message));
  MOCK_METHOD0(getCurrentOffset, int64_t());
  MOCK_METHOD0(flushSendQueue, void());
};
