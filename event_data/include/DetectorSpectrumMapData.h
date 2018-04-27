#pragma once

#include "df12_det_spec_map_generated.h"

class DetectorSpectrumMapData {
public:
  DetectorSpectrumMapData() = default;
  explicit DetectorSpectrumMapData(const std::string &filename);

  flatbuffers::unique_ptr_t getBufferPointer(std::string &buffer);
  void decodeMessage(const uint8_t *buf);

  int32_t getNumberOfEntries() { return m_numberOfEntries; }
  std::vector<int32_t> getDetectors() { return m_detectors; }
  std::vector<int32_t> getSpectra() { return m_spectra; }
  size_t getBufferSize() { return m_bufferSize; }

  void setNumberOfEntries(int32_t numberOfEntries) {
    m_numberOfEntries = numberOfEntries;
  }

private:
  void readFile(const std::string &filename);
  int32_t m_numberOfEntries = 0;
  size_t m_bufferSize = 0;
  std::vector<int32_t> m_detectors;
  std::vector<int32_t> m_spectra;
};
