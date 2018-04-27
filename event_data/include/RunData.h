#pragma once

#include "ba57_run_info_generated.h"

class RunData {

public:
  bool decodeMessage(const uint8_t *buf);
  flatbuffers::unique_ptr_t getRunStartBufferPointer(std::string &buffer);
  flatbuffers::unique_ptr_t getRunStopBufferPointer(std::string &buffer);

  void setRunNumber(int32_t runNumber) { m_runNumber = runNumber; }
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

  int32_t getRunNumber() { return m_runNumber; }
  std::string getInstrumentName() { return m_instrumentName; }
  uint64_t getStartTime() { return m_startTime; }
  uint64_t getStopTime() { return m_stopTime; }
  size_t getBufferSize() { return m_bufferSize; }
  int32_t getNumberOfPeriods() { return m_numberOfPeriods; }

  std::string runInfo();

private:
  uint64_t timeStringToUint64(const std::string &inputTime);
  uint64_t secondsToNanoseconds(time_t timeInSeconds);

  uint64_t m_startTime = 0;
  uint64_t m_stopTime = 0;
  int32_t m_runNumber = 0;
  std::string m_instrumentName = "";
  size_t m_bufferSize = 0;
  int32_t m_numberOfPeriods = 0;
};
