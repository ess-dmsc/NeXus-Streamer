#include <iostream>

#include "KafkaEventPublisher.h"

/**
 * Set up the configuration for the publisher and initialise it
 *
 * @param broker_str - the IP or hostname of the broker
 * @param topic_str - the name of the datastream (topic) to publish the data to
 */
void KafkaEventPublisher::setUp(const std::string &broker,
                                const std::string &instrumentName) {

  std::cout << "Setting up Kafka producer" << std::endl;

  std::string error_str;

  auto conf = std::unique_ptr<RdKafka::Conf>(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  auto tconf = std::shared_ptr<RdKafka::Conf>(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  conf->set("metadata.broker.list", broker, error_str);
  conf->set("message.send.max.retries", "3", error_str);
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
      RdKafka::Producer::create(conf.get(), error_str));
  if (!m_producer_ptr.get()) {
    std::cerr << "Failed to create producer: " << error_str << std::endl;
    exit(1);
  }

  // Create topics
  m_topic_ptr = createTopicHandle(instrumentName, "_events", tconf);
  m_runTopic_ptr = createTopicHandle(instrumentName, "_runInfo", tconf);
  m_detSpecTopic_ptr = createTopicHandle(instrumentName, "_detSpecMap", tconf);
  m_sampleEnvTopic_ptr = createTopicHandle(instrumentName, "_sampleEnv", tconf);

  // This ensures everything is ready when we need to query offset information
  // later
  m_producer_ptr->poll(1000);
}

/**
 * Create a topic handle
 *
 * @param topic_str : name of the topic
 * @param topicConfig : configuration of the topic
 * @return topic handle
 */
std::shared_ptr<RdKafka::Topic> KafkaEventPublisher::createTopicHandle(
    const std::string &topicPrefix, const std::string &topicSuffix,
    std::shared_ptr<RdKafka::Conf> topicConfig) {
  std::string topic_str = topicPrefix + topicSuffix;
  std::string error_str;
  auto topic_ptr = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(
      m_producer_ptr.get(), topic_str, topicConfig.get(), error_str));
  if (!topic_ptr.get()) {
    std::cerr << "Failed to create topic: " << error_str << std::endl;
    exit(1);
  }
  return topic_ptr;
}

/**
 * Publish the provided message to the datastream
 *
 * @param buf - pointer to the message buffer
 * @param messageSize - the size of the message in bytes
 */
void KafkaEventPublisher::sendEventMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_topic_ptr);
}

void KafkaEventPublisher::sendRunMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_runTopic_ptr);
}

void KafkaEventPublisher::sendDetSpecMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_detSpecTopic_ptr);
}

void KafkaEventPublisher::sendSampleEnvMessage(char *buf, size_t messageSize) {
  sendMessage(buf, messageSize, m_sampleEnvTopic_ptr);
}

void KafkaEventPublisher::sendMessage(char *buf, size_t messageSize,
                                      std::shared_ptr<RdKafka::Topic> topic) {
  RdKafka::ErrorCode resp;
  do {

    resp = m_producer_ptr->produce(topic.get(), m_partitionNumber,
                                   RdKafka::Producer::RK_MSG_COPY, buf,
                                   messageSize, NULL, NULL);

    if (resp != RdKafka::ERR_NO_ERROR) {
      if (resp != RdKafka::ERR__QUEUE_FULL) {
        std::cerr << "% Produce failed: " << RdKafka::err2str(resp)
                  << std::endl;
        std::cerr << "message size was " << messageSize << std::endl;
      }
      // This blocking poll call should give Kafka some time for the problem to
      // be resolved
      // for example for messages to leave the queue if it is full
      m_producer_ptr->poll(1000);
    } else {
      m_producer_ptr->poll(0);
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
