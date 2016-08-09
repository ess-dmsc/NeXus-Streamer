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
  bool getEventDetIds(std::vector<uint32_t> &detIds, hsize_t frameNumber);
  bool getEventTofs(std::vector<uint64_t> &tofs, hsize_t frameNumber);
  size_t getNumberOfFrames() { return m_numberOfFrames; };

private:
  hsize_t getFrameStart(hsize_t frameNumber);
  hsize_t getNumberOfEventsInFrame(hsize_t frameNumber);
  H5FilePtr m_file = nullptr;
  size_t m_numberOfFrames;
};

#endif // ISIS_NEXUS_STREAMER_NEXUSFILEREADER_H
