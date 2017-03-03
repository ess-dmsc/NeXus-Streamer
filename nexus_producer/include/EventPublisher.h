#ifndef ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H
#define ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H

#include <string>

class EventPublisher {
public:
  virtual ~EventPublisher() {}
  virtual void setUp(const std::string &broker,
                     const std::string &instrumentName) = 0;
  virtual void sendEventMessage(char *buf, size_t messageSize) = 0;
  virtual void sendRunMessage(char *buf, size_t messageSize) = 0;
  virtual void sendDetSpecMessage(char *buf, size_t messageSize) = 0;
  virtual int64_t getCurrentOffset() = 0;
};

#endif // ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H
