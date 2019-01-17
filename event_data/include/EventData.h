#pragma once

#include <cstdint>
#include <vector>

#include "../../core/include/Message.h"
#include "ev42_events_generated.h"
#include "is84_isis_events_generated.h"

class EventData {

public:
  // Construct a new empty EventData object
  EventData() = default;

  // Decode message into existing EventData instance
  bool decodeMessage(const uint8_t *buf);

  // Setters
  void setDetId(std::vector<uint32_t> detIds) { m_detId = std::move(detIds); }
  void setTof(std::vector<uint32_t> tofs) { m_tof = std::move(tofs); }
  void setTotalCounts(uint64_t totalCounts) { m_totalCounts = totalCounts; }
  void setProtonCharge(float protonCharge) { m_protonCharge = protonCharge; }
  void setPeriod(uint32_t period) { m_period = period; }
  void setFrameTime(uint64_t frameTime) { m_frameTime = frameTime; }

  // Getters
  std::vector<uint32_t> getDetId() { return m_detId; }
  std::vector<uint32_t> getTof() { return m_tof; }
  uint32_t getNumberOfEvents() { return static_cast<uint32_t>(m_tof.size()); }
  uint64_t getTotalCounts() { return m_totalCounts; }
  float getProtonCharge() { return m_protonCharge; }
  uint32_t getPeriod() { return m_period; }
  uint64_t getFrameTime() { return m_frameTime; }

  Streamer::Message getBuffer(uint64_t messageID);

private:
  // Default values here should match default values in the schema
  // if the values are then used to create the buffer they are omitted by
  // flatbuffers to reduce the message size
  std::vector<uint32_t> m_detId = {};
  std::vector<uint32_t> m_tof = {};
  uint64_t m_totalCounts = 0;
  uint64_t m_frameTime = 0;
  float m_protonCharge = -1;
  uint32_t m_period = std::numeric_limits<uint32_t>::max();
};
