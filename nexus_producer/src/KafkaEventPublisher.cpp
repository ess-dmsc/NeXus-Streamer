#include <iostream>

#include "KafkaEventPublisher.h"

void KafkaEventPublisher::setUp(const std::string &broker_str,
                                const std::string &topic_str) {
  std::string error_str;

  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  conf->set("metadata.broker.list", broker_str, error_str);

  // Create producer
  m_producer_ptr = std::unique_ptr<RdKafka::Producer>(
      RdKafka::Producer::create(conf, error_str));
  if (!m_producer_ptr.get()) {
    std::cerr << "Failed to create producer: " << error_str << std::endl;
    exit(1);
  }

  // Create topic handle
  m_topic_ptr = std::unique_ptr<RdKafka::Topic>(RdKafka::Topic::create(
      m_producer_ptr.get(), topic_str, tconf, error_str));
  if (!m_topic_ptr.get()) {
    std::cerr << "Failed to create topic: " << error_str << std::endl;
    exit(1);
  }
}

void KafkaEventPublisher::sendMessage(char *buf, size_t messageSize) {
  RdKafka::ErrorCode resp = m_producer_ptr->produce(
      m_topic_ptr.get(), 0, RdKafka::Producer::RK_MSG_COPY, buf, messageSize,
      NULL, NULL);
  std::cout << "Sending buffer with size: " << messageSize << std::endl;

  if (resp != RdKafka::ERR_NO_ERROR)
    std::cerr << "% Produce failed: " << RdKafka::err2str(resp) << std::endl;

  m_producer_ptr->poll(0);
}