#pragma once

#include "../../event_data/include/SampleEnvironmentEvent.h"
#include <cmath>
#include <h5cpp/hdf5.hpp>
#include <unordered_map>

using sEEventVector = std::vector<std::shared_ptr<SampleEnvironmentEvent>>;

struct EventDataFrame {
  EventDataFrame(std::vector<uint32_t> detIDs, std::vector<uint32_t> tofs)
      : detectorIDs(std::move(detIDs)), timeOfFlights(std::move(tofs)) {}
  std::vector<uint32_t> detectorIDs;
  std::vector<uint32_t> timeOfFlights;
};

struct HistogramData {
  HistogramData(std::vector<int32_t> detectorCounts,
                std::vector<float> tofBinEdges)
      : counts(std::move(detectorCounts)),
        timeOfFlight(std::move(tofBinEdges)) {}
  std::vector<int32_t> counts;
  std::vector<float> timeOfFlight;
};

class FileReader {
public:
  virtual ~FileReader() = default;

  virtual hsize_t getFileSize() = 0;
  virtual uint64_t getTotalEventCount() = 0;
  virtual uint32_t getPeriodNumber() = 0;
  virtual float getProtonCharge(hsize_t frameNumber) = 0;
  virtual std::vector<EventDataFrame> getEventData(hsize_t frameNumber) = 0;
  virtual std::vector<HistogramData> getHistoData() = 0;
  virtual size_t getNumberOfFrames() = 0;
  virtual hsize_t getNumberOfEventsInFrame(hsize_t frameNumber,
                                           size_t eventGroupNumber) = 0;
  virtual uint64_t getFrameTime(hsize_t frameNumber) = 0;
  virtual std::string getInstrumentName() = 0;
  virtual std::unordered_map<hsize_t, sEEventVector> getSEEventMap() = 0;
  virtual int32_t getNumberOfPeriods() = 0;
  virtual uint64_t getRelativeFrameTimeMilliseconds(hsize_t frameNumber) = 0;
  virtual bool isISISFile() = 0;
  virtual uint64_t getTotalEventsInGroup(size_t eventGroupNumber) = 0;
};
