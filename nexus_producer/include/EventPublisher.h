#ifndef ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H
#define ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H

#include <string>

class EventPublisher {
public:
  virtual ~EventPublisher() {}
  virtual void setUp(const std::string &broker, const std::string &topic) = 0;
  virtual void sendMessage(char *buf, size_t messageSize) = 0;
  virtual int64_t getCurrentOffset() = 0;
};

#endif // ISIS_NEXUS_STREAMER_EVENTPUBLISHER_H
