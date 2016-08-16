#ifndef ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H
#define ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H

#include "EventPublisher.h"
#include <gmock/gmock.h>

class MockEventPublisher : public EventPublisher {
public:
  MOCK_METHOD4(setUp, void(const std::string &broker, const std::string &topic,
                           const std::string &runTopic,
                           const std::string &detSpecTopic));
  MOCK_METHOD2(sendRunMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD2(sendDetSpecMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD2(sendEventMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD0(getCurrentOffset, int64_t());
};

#endif // ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H
