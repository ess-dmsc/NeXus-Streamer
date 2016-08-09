#ifndef ISIS_NEXUS_STREAMER_EVENTDATA_H
#define ISIS_NEXUS_STREAMER_EVENTDATA_H

#include <cstdint>
#include <vector>

#include "event_schema_generated.h"

class EventData {

public:
  // Construct a new empty EventData object
  EventData(){};
  // Construct an EventData object from a flatbuf buffer
  EventData(const uint8_t *buf);

  // Decode message into existing EventData instance
  void decodeMessage(const uint8_t *buf);

  // Setters
  void setDetId(std::vector<int32_t> detIds) { m_detId = detIds; };
  void setTof(std::vector<float> tofs) { m_tof = tofs; };
  void setFrameNumber(int32_t frameNumber) { m_frameNumber = frameNumber; };
  void setTotalCounts(uint64_t totalCounts) { m_totalCounts = totalCounts; };

  // Getters
  std::vector<int32_t> getDetId() { return m_detId; };
  std::vector<float> getTof() { return m_tof; };
  uint32_t getFrameNumber() { return m_frameNumber; };
  uint32_t getNumberOfEvents() { return m_tof.size(); };
  uint64_t getTotalCounts() { return m_totalCounts; };

  flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer);

  size_t getBufferSize() { return m_bufferSize; };

private:

  // Default values here should match default values in the schema
  // if the values are then used to create the buffer they are omitted by
  // flatbuffers to reduce the message size
  std::vector<int32_t> m_detId = {};
  std::vector<float> m_tof = {};
  int32_t m_frameNumber = 0;
  uint64_t m_totalCounts = 0;
  size_t m_bufferSize = 0;
  float m_frameTime = 0;
  float m_protonCharge = 0;
  int32_t m_period = 0;
  bool m_endOfFrame = false;
  bool m_endOfRun = false;
};

#endif // ISIS_NEXUS_STREAMER_EVENTDATA_H
