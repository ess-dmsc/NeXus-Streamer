#ifndef ISIS_NEXUS_STREAMER_NEXUSFILEREADER_H
#define ISIS_NEXUS_STREAMER_NEXUSFILEREADER_H

#include <H5Cpp.h>
#include <memory>
#include <vector>

// a typedef for our managed H5File pointer
typedef std::unique_ptr<H5::H5File> H5FilePtr;

class NexusFileReader {
public:
  NexusFileReader(const std::string &filename);

  hsize_t getFileSize();
  uint64_t getTotalEventCount();
  int32_t getPeriodNumber();
  float getProtonCharge(hsize_t frameNumber);
  bool getEventDetIds(std::vector<int32_t> &detIds, hsize_t frameNumber);
  bool getEventTofs(std::vector<float> &tofs, hsize_t frameNumber);
  size_t getNumberOfFrames() { return m_numberOfFrames; };
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber);
  double getFrameTime(hsize_t frameNumber);
  int64_t getRunStartTime();
  std::string getInstrumentName();
  std::vector<std::string> getNamesInGroup(const std::string &groupName);

private:
  template <typename T>
  T getSingleValueFromDataset(const std::string &dataset, H5::PredType datatype,
                              hsize_t offset);
  hsize_t getFrameStart(hsize_t frameNumber);
  H5FilePtr m_file = nullptr;
  size_t m_numberOfFrames;
  int64_t convertStringToUnixTime(const std::string &timeString);
};

#endif // ISIS_NEXUS_STREAMER_NEXUSFILEREADER_H
