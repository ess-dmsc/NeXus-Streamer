#include "KafkaPublisher.h"
#include "../../core/include/Message.h"
#include "TopicNames.h"

KafkaPublisher::~KafkaPublisher() {
  flushSendQueue();
  RdKafka::wait_destroyed(5000);
}

/**
 * Set up the configuration for the publisher and initialise it
 *
 * @param broker_str - the IP or hostname of the broker
 * @param topic_str - the name of the datastream (topic) to publish the data to
 */
void KafkaPublisher::setUp(const std::string &broker,
                           const std::string &instrumentName) {

  m_logger->info("Setting up Kafka producer");

  std::string error_str;

  auto conf = std::unique_ptr<RdKafka::Conf>(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  auto tconf = std::shared_ptr<RdKafka::Conf>(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  std::string const maxMessageSize = "100000000";

  conf->set("metadata.broker.list", broker, error_str);
  conf->set("message.send.max.retries", "3", error_str);
  conf->set("message.max.bytes", maxMessageSize, error_str);
  conf->set("fetch.message.max.bytes", maxMessageSize, error_str);
  conf->set("replica.fetch.max.bytes", maxMessageSize, error_str);
  conf->set("api.version.request", "true", error_str);

  if (!m_compression.empty()) {
    if (conf->set("compression.codec", m_compression, error_str) !=
        RdKafka::Conf::CONF_OK) {
      m_logger->error(error_str);
      exit(1);
    }
    m_logger->info("Using {} compression codec", m_compression);
  }

  // Create producer
  m_producer_ptr = std::shared_ptr<RdKafka::Producer>(
      RdKafka::Producer::create(conf.get(), error_str));
  if (m_producer_ptr == nullptr) {
    m_logger->error("Failed to create producer: {}", error_str);
    throw std::runtime_error("Failed to create producer");
  }

  // Create topics
  auto topicNames = TopicNames(instrumentName);
  m_topic_ptr = createTopicHandle(topicNames.event, tconf);
  m_runTopic_ptr = createTopicHandle(topicNames.runInfo, tconf);
  m_detSpecTopic_ptr = createTopicHandle(topicNames.detSpecMap, tconf);
  m_sampleEnvTopic_ptr = createTopicHandle(topicNames.sampleEnv, tconf);
  m_histogramTopic_ptr = createTopicHandle(topicNames.histogram, tconf);

  // This ensures everything is ready when we need to query offset information
  // later
  m_producer_ptr->poll(1000);
}

/**
 * Wait for all messages in the current producer queue to be published
 */
void KafkaPublisher::flushSendQueue() {
  auto error = m_producer_ptr->flush(2000);
  if (error != RdKafka::ERR_NO_ERROR) {
    m_logger->error("Producer queue flush failed.");
  }
}

/**
 * Create a topic handle
 *
 * @param topic_str - name of the topic
 * @param topicConfig - configuration of the topic
 * @return topic handle
 */
std::shared_ptr<RdKafka::Topic>
KafkaPublisher::createTopicHandle(const std::string &topicName,
                                  std::shared_ptr<RdKafka::Conf> topicConfig) {
  std::string error_str;
  auto topic_ptr = std::shared_ptr<RdKafka::Topic>(RdKafka::Topic::create(
      m_producer_ptr.get(), topicName, topicConfig.get(), error_str));
  if (topic_ptr == nullptr) {
    m_logger->error("Failed to create topic: {}", error_str);
    throw std::runtime_error("Failed to create topic");
  }
  return topic_ptr;
}

/**
 * Publish the provided message to the datastream
 *
 * @param buf - pointer to the message buffer
 * @param messageSize - the size of the message in bytes
 */
void KafkaPublisher::sendEventMessage(Streamer::Message &message) {
  sendMessage(message, m_topic_ptr);
}

void KafkaPublisher::sendRunMessage(Streamer::Message &message) {
  sendMessage(message, m_runTopic_ptr);
}

void KafkaPublisher::sendDetSpecMessage(Streamer::Message &message) {
  sendMessage(message, m_detSpecTopic_ptr);
}

void KafkaPublisher::sendSampleEnvMessage(Streamer::Message &message) {
  sendMessage(message, m_sampleEnvTopic_ptr);
}

void KafkaPublisher::sendHistogramMessage(Streamer::Message &message) {
  sendMessage(message, m_histogramTopic_ptr);
}

void KafkaPublisher::sendMessage(Streamer::Message &message,
                                 std::shared_ptr<RdKafka::Topic> topic) {
  RdKafka::ErrorCode resp;
  do {

    resp = m_producer_ptr->produce(
        topic.get(), m_partitionNumber, RdKafka::Producer::RK_MSG_COPY,
        message.data(), message.size(), nullptr, nullptr);

    if (resp != RdKafka::ERR_NO_ERROR) {
      if (resp != RdKafka::ERR__QUEUE_FULL) {
        m_logger->error("Produce failed: {}\n"
                        "Message size was: {}",
                        RdKafka::err2str(resp), message.size());
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

int64_t KafkaPublisher::getCurrentOffset() {

  int64_t lowOffset = 0;
  int64_t highOffset = 0;
  auto err = m_producer_ptr->query_watermark_offsets(
      m_topic_ptr->name(), m_partitionNumber, &lowOffset, &highOffset, -1);
  if (err != RdKafka::ERR_NO_ERROR) {
    m_logger->error("Failed to acquire current offset, will use 0: {}",
                    RdKafka::err2str(err));
    return 0;
  }
  return highOffset;
}
