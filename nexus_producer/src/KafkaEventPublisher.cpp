#include <iostream>

#include "KafkaEventPublisher.h"

/**
 * Set up the configuration for the publisher and initialise it
 *
 * @param broker_str - the IP or hostname of the broker
 * @param topic_str - the name of the datastream (topic) to publish the data to
 */
void KafkaEventPublisher::setUp(const std::string &broker_str,
                                const std::string &topic_str,
                                const std::string &runTopic_str) {

  std::cout << "Setting up Kafka producer" << std::endl;

  std::string error_str;

  RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  RdKafka::Conf *tconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

  conf->set("metadata.broker.list", broker_str, error_str);
  conf->set("message.max.bytes", "10000000", error_str);
  conf->set("fetch.message.max.bytes", "10000000", error_str);
  conf->set("replica.fetch.max.bytes", "10000000", error_str);
  if (!m_compression.empty()) {
    if (conf->set("compression.codec", m_compression, error_str) !=
        RdKafka::Conf::CONF_OK) {
      std::cerr << error_str << std::endl;
      exit(1);
    }
    std::cout << "Using " << m_compression << " compression codec" << std::endl;
  }

  // Create producer
  m_producer_ptr = std::shared_ptr<RdKafka::Producer>(
      RdKafka::Producer::create(conf, error_str));
  if (!m_producer_ptr.get()) {
    std::cerr << "Failed to create producer: " << error_str << std::endl;
    exit(1);
  }

  // Create topic handle
  m_topic_ptr = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(
      m_producer_ptr.get(), topic_str, tconf, error_str));
  if (!m_topic_ptr.get()) {
    std::cerr << "Failed to create topic: " << error_str << std::endl;
    exit(1);
  }

  // Create run message producer
  m_runProducer_ptr = std::shared_ptr<RdKafka::Producer>(
      RdKafka::Producer::create(conf, error_str));
  if (!m_runProducer_ptr.get()) {
    std::cerr << "Failed to create producer: " << error_str << std::endl;
    exit(1);
  }

  // Create run topic handle
  m_runTopic_ptr = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(
      m_runProducer_ptr.get(), runTopic_str, tconf, error_str));
  if (!m_runTopic_ptr.get()) {
    std::cerr << "Failed to create topic: " << error_str << std::endl;
    exit(1);
  }

  // This ensures everything is ready when we need to query offset information later
  m_producer_ptr->poll(1000);
}

/**
 * Publish the provided message to the datastream
 *
 * @param buf - pointer to the message buffer
 * @param messageSize - the size of the message in bytes
 */
void KafkaEventPublisher::sendEventMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_producer_ptr, m_topic_ptr);
}

void KafkaEventPublisher::sendRunMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_runProducer_ptr, m_runTopic_ptr);
}

void KafkaEventPublisher::sendMessage(
    char *buf, size_t messageSize, std::shared_ptr<RdKafka::Producer> producer,
    std::shared_ptr<RdKafka::Topic> topic) {
  RdKafka::ErrorCode resp;
  do {

    resp = producer->produce(topic.get(), m_partitionNumber,
                             RdKafka::Producer::RK_MSG_COPY, buf, messageSize,
                             NULL, NULL);

    if (resp != RdKafka::ERR_NO_ERROR) {
      if (resp != RdKafka::ERR__QUEUE_FULL) {
        std::cerr << "% Produce failed: " << RdKafka::err2str(resp)
                  << std::endl;
        std::cerr << "message size was " << messageSize << std::endl;
      }
      // This blocking poll call should give Kafka some time for the problem to
      // be resolved
      // for example for messages to leave the queue if it is full
      producer->poll(1000);
    } else {
      producer->poll(0);
    }
  } while (resp == RdKafka::ERR__QUEUE_FULL);
}

int64_t KafkaEventPublisher::getCurrentOffset() {

  int64_t lowOffset = 0;
  int64_t highOffset = 0;
  auto err = m_producer_ptr->query_watermark_offsets(
      m_topic_ptr->name(), m_partitionNumber, &lowOffset, &highOffset, -1);
  if (err != RdKafka::ERR_NO_ERROR) {
    std::cerr << "%% Failed to acquire current offset, will use 0: "
              << RdKafka::err2str(err) << std::endl;
    return 0;
  }
  return highOffset;
}
