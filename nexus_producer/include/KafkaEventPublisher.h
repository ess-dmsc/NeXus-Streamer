#pragma once

#include <librdkafka/rdkafkacpp.h>
#include <memory>
#include <spdlog/spdlog.h>

#include "EventPublisher.h"

class KafkaEventPublisher : public EventPublisher {
public:
  KafkaEventPublisher() = default;
  explicit KafkaEventPublisher(std::string compression)
      : m_compression(std::move(compression)){};
  ~KafkaEventPublisher() override;

  std::shared_ptr<RdKafka::Topic>
  createTopicHandle(const std::string &topicPrefix,
                    const std::string &topicSuffix,
                    std::shared_ptr<RdKafka::Conf> tconf);
  void setUp(const std::string &broker,
             const std::string &instrumentName) override;
  void sendEventMessage(Streamer::Message &message) override;
  void sendRunMessage(Streamer::Message &message) override;
  void sendDetSpecMessage(Streamer::Message &message) override;
  void sendSampleEnvMessage(Streamer::Message &message) override;
  int64_t getCurrentOffset() override;
  void flushSendQueue() override;

private:
  void sendMessage(Streamer::Message &message,
                   std::shared_ptr<RdKafka::Topic> topic);

  std::shared_ptr<RdKafka::Producer> m_producer_ptr;
  std::shared_ptr<RdKafka::Topic> m_topic_ptr;
  std::shared_ptr<RdKafka::Topic> m_runTopic_ptr;
  std::shared_ptr<RdKafka::Topic> m_detSpecTopic_ptr;
  std::shared_ptr<RdKafka::Topic> m_sampleEnvTopic_ptr;
  std::string m_compression = "";
  std::shared_ptr<spdlog::logger> m_logger = spdlog::get("LOG");

  // Use default partition assignment for messages
  int m_partitionNumber = RdKafka::Topic::PARTITION_UA;
};
