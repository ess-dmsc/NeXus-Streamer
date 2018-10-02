#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include <h5cpp/hdf5.hpp>
#include <unordered_map>
#include <cmath>

using sEEventVector = std::vector<std::shared_ptr<SampleEnvironmentEvent>>;

class FileReader {
public:
  virtual ~FileReader() = 0;

  virtual hsize_t getFileSize() = 0;
  virtual uint64_t getTotalEventCount() = 0;
  virtual uint32_t getPeriodNumber() = 0;
  virtual float getProtonCharge(hsize_t frameNumber) = 0;
  virtual bool getEventDetIds(std::vector<uint32_t> &detIds,
                              hsize_t frameNumber) = 0;
  virtual bool getEventTofs(std::vector<uint32_t> &tofs,
                            hsize_t frameNumber) = 0;
  virtual size_t getNumberOfFrames() = 0;
  virtual hsize_t getNumberOfEventsInFrame(hsize_t frameNumber) = 0;
  virtual uint64_t getFrameTime(hsize_t frameNumber) = 0;
  virtual std::string getInstrumentName() = 0;
  virtual std::unordered_map<hsize_t, sEEventVector> getSEEventMap() = 0;
  virtual int32_t getNumberOfPeriods() = 0;
  virtual uint64_t getRelativeFrameTimeMilliseconds(hsize_t frameNumber) = 0;
};
