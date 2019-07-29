#pragma once

#include "../../core/include/Message.h"
#include "y2gw_run_info_generated.h"

class RunData {

public:
  bool decodeMessage(const uint8_t *buf);
  Streamer::Message getRunStartBuffer();
  Streamer::Message getRunStopBuffer();

  void setRunID(const std::string &runID) { m_runID = runID; }
  void setInstrumentName(const std::string &instrumentName) {
    m_instrumentName = instrumentName;
  }
  void setStartTime(const std::string &inputTime);
  void setStopTime(const std::string &inputTime);
  void setStartTimeInSeconds(time_t inputTime);
  void setStartTimeInNanoseconds(uint64_t inputTime) {
    m_startTime = inputTime;
  };
  void setStopTime(uint64_t inputTime) { m_stopTime = inputTime; }
  void setStartTime(uint64_t inputTime) { m_startTime = inputTime; }
  void setNumberOfPeriods(int32_t numberOfPeriods) {
    m_numberOfPeriods = numberOfPeriods;
  }
  void setNexusStructure(const std::string &nexusStructure) {
    m_nexusStructure = nexusStructure;
  };

  std::string getRunID() { return m_runID; }
  std::string getInstrumentName() { return m_instrumentName; }
  uint64_t getStartTime() { return m_startTime; }
  uint64_t getStopTime() { return m_stopTime; }
  int32_t getNumberOfPeriods() { return m_numberOfPeriods; }
  std::string getNexusStructure() { return m_nexusStructure; };

  std::string runInfo();

private:
  uint64_t timeStringToUint64(const std::string &inputTime);
  uint64_t secondsToNanoseconds(time_t timeInSeconds);

  uint64_t m_startTime = 0;
  uint64_t m_stopTime = 0;
  std::string m_runID;
  std::string m_instrumentName;
  int32_t m_numberOfPeriods = 0;
  std::string m_nexusStructure;
};
