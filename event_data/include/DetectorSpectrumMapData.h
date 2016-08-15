#ifndef ISIS_NEXUS_STREAMER_FOR_MANTID_DETECTORSPECTRUMMAPDATA_H
#define ISIS_NEXUS_STREAMER_FOR_MANTID_DETECTORSPECTRUMMAPDATA_H

#include "det_spec_mapping_schema_generated.h"

class DetectorSpectrumMapData {
public:
  DetectorSpectrumMapData(const std::string &filename);

  size_t getNumberOfEntries() { return m_numberOfEntries; }
  std::vector<int32_t> getDetectors() { return m_detectors; }
  std::vector<int32_t> getSpectra() { return m_spectra; }

private:
  void readFile(const std::string &filename);
  size_t m_numberOfEntries = 0;
  std::vector<int32_t> m_detectors;
  std::vector<int32_t> m_spectra;
};

#endif // ISIS_NEXUS_STREAMER_FOR_MANTID_DETECTORSPECTRUMMAPDATA_H
