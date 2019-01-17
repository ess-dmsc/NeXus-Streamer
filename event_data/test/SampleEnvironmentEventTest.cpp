#include "SampleEnvironmentEvent.h"
#include "SampleEnvironmentEventDouble.h"
#include "SampleEnvironmentEventInt.h"
#include "SampleEnvironmentEventLong.h"
#include <gtest/gtest.h>

class SampleEnvironmentEventTest : public ::testing::Test {
public:
  template <typename T>
  void decodeSampleEnvMessage(Streamer::Message &messageBuffer,
                              const std::string &inputName, T inputValue) {
    auto messageData =
        GetLogData(reinterpret_cast<const uint8_t *>(messageBuffer.data()));
    std::string name = messageData->source_name()->str();

    EXPECT_EQ(inputName, name);

    if (messageData->value_type() == Value::Int) {
      auto value = static_cast<const Int *>(messageData->value());
      EXPECT_EQ(inputValue, value->value());
    } else if (messageData->value_type() == Value::Long) {
      auto value = static_cast<const Long *>(messageData->value());
      EXPECT_EQ(inputValue, value->value());
    } else if (messageData->value_type() == Value::Double) {
      auto value = static_cast<const Double *>(messageData->value());
      EXPECT_EQ(inputValue, value->value());
    } else {
      throw std::runtime_error(
          "Unexpected PV type which is not supported by this client");
    }
  }
};

TEST_F(SampleEnvironmentEventTest, create_int_event) {
  auto value = std::numeric_limits<int32_t>::max();
  int64_t runStart = 0;
  EXPECT_NO_THROW(SampleEnvironmentEventInt("TEMP_1", 0.242, value, runStart));
}

TEST_F(SampleEnvironmentEventTest, get_int_event) {
  auto value = std::numeric_limits<int32_t>::max();
  int64_t runStart = 0;
  std::string name = "TEMP_1";
  auto intEvent = SampleEnvironmentEventInt(name, 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(intEvent.getSEEvent(builder));

  // Create message
  auto buffer = intEvent.getBuffer();

  // Test decoded message
  decodeSampleEnvMessage(buffer, name, value);
}

TEST_F(SampleEnvironmentEventTest, create_long_event) {
  auto value = std::numeric_limits<int64_t>::max();
  int64_t runStart = 0;
  EXPECT_NO_THROW(SampleEnvironmentEventLong("TEMP_1", 0.242, value, runStart));
}

TEST_F(SampleEnvironmentEventTest, get_long_event) {
  auto value = std::numeric_limits<int64_t>::max();
  int64_t runStart = 0;
  std::string name = "TEMP_1";
  auto longEvent = SampleEnvironmentEventLong(name, 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(longEvent.getSEEvent(builder));

  // Create message
  auto buffer = longEvent.getBuffer();

  // Test decoded message
  decodeSampleEnvMessage(buffer, name, value);
}

TEST_F(SampleEnvironmentEventTest, create_double_event) {
  double value = 42.12;
  int64_t runStart = 0;
  EXPECT_NO_THROW(
      SampleEnvironmentEventDouble("TEMP_1", 0.242, value, runStart));
}

TEST_F(SampleEnvironmentEventTest, get_double_event) {
  double value = 42.12;
  int64_t runStart = 0;
  std::string name = "TEMP_1";
  auto doubleEvent = SampleEnvironmentEventDouble(name, 0.242, value, runStart);
  flatbuffers::FlatBufferBuilder builder;
  EXPECT_NO_THROW(doubleEvent.getSEEvent(builder));

  // Create message
  auto buffer = doubleEvent.getBuffer();

  // Test decoded message
  decodeSampleEnvMessage(buffer, name, value);
}
