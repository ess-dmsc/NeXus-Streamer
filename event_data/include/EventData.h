#ifndef ISIS_NEXUS_STREAMER_EVENTDATA_H
#define ISIS_NEXUS_STREAMER_EVENTDATA_H

#include <cstdint>
#include <vector>

#include "ev42_events_generated.h"

uint64_t getMessageID(const std::string &rawbuf);

class EventData {

public:
  // Construct a new empty EventData object
  EventData() {}

  // Decode message into existing EventData instance
  bool decodeMessage(const uint8_t *buf);

  // Setters
  void setDetId(std::vector<uint32_t> detIds) { m_detId = detIds; }
  void setTof(std::vector<uint32_t> tofs) { m_tof = tofs; }
  void setTotalCounts(uint64_t totalCounts) { m_totalCounts = totalCounts; }
  void setProtonCharge(float protonCharge) { m_protonCharge = protonCharge; }
  void setPeriod(uint32_t period) { m_period = period; }
  void setFrameTime(uint64_t frameTime) { m_frameTime = frameTime; }

  // Getters
  std::vector<uint32_t> getDetId() { return m_detId; }
  std::vector<uint32_t> getTof() { return m_tof; }
  uint32_t getNumberOfEvents() { return m_tof.size(); }
  uint64_t getTotalCounts() { return m_totalCounts; }
  float getProtonCharge() { return m_protonCharge; }
  uint32_t getPeriod() { return m_period; }
  uint64_t getFrameTime() { return m_frameTime; }
  size_t getBufferSize() { return m_bufferSize; }

  flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer,
                                             uint64_t messageID);

private:
  // Default values here should match default values in the schema
  // if the values are then used to create the buffer they are omitted by
  // flatbuffers to reduce the message size
  std::vector<uint32_t> m_detId = {};
  std::vector<uint32_t> m_tof = {};
  uint64_t m_totalCounts = 0;
  size_t m_bufferSize = 0;
  uint64_t m_frameTime = 0;
  float m_protonCharge = 0;
  uint32_t m_period = 0;
};

#endif // ISIS_NEXUS_STREAMER_EVENTDATA_H
