#ifndef ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H
#define ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H

#include "EventPublisher.h"
#include <gmock/gmock.h>

class MockEventPublisher : public EventPublisher {
public:
  MOCK_METHOD2(setUp,
               void(const std::string &broker, const std::string &topic));
  MOCK_METHOD2(sendMessage, void(char *buf, size_t messageSize));
  MOCK_METHOD0(getCurrentOffset, int64_t());
};

#endif // ISIS_NEXUS_STREAMER_MOCKEVENTPUBLISHER_H
