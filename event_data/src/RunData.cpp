#include "RunData.h"

#include <sstream>
#include <iostream>
#include <iomanip>

void RunData::setStartTime(int32_t startTime) {
  const time_t sTime = static_cast<time_t>(startTime);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&sTime), "%Y-%m-%dT%H:%M:%S");
  m_startTime = ss.str();
}
