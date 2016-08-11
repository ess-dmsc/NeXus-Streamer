#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_RUNDATA_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_RUNDATA_H

#include "event_schema_generated.h"

class RunData {

public:
  // bool decodeMessage(const uint8_t *buf);
  // flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer);

  void setRunNumber(int32_t runNumber) { m_runNumber = runNumber; }
  void setInstrumentName(const std::string &instrumentName) {
    m_instrumentName = instrumentName;
  }
  void setStartTime(int32_t startTime);

  int32_t getRunNumber() { return m_runNumber; }
  std::string getInstrumentName() { return m_instrumentName; }
  std::string getStartTime() { return m_startTime; }

private:
  std::string m_startTime = "";
  int32_t m_runNumber = 0;
  std::string m_instrumentName = "";
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_RUNDATA_H
