#include <gtest/gtest.h>

#include "../include/DetectorSpectrumMapData.h"

class DetectorSpectrumMapDataTest : public ::testing::Test {};

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_file) {
  extern std::string testDataPath;
  EXPECT_NO_THROW(DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat"));
}

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_detectors) {
  extern std::string testDataPath;
  auto detSpecMap = DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");
  EXPECT_EQ(245768, detSpecMap.getNumberOfEntries());

  auto detectors = detSpecMap.getDetectors();
  EXPECT_EQ(1, detectors[0]);
  EXPECT_EQ(1100000, detectors[8]);
  EXPECT_EQ(4523511, detectors[245767]);
}

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_spectra) {
  extern std::string testDataPath;
  auto detSpecMap = DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");
  EXPECT_EQ(245768, detSpecMap.getNumberOfEntries());

  auto spectra = detSpecMap.getSpectra();
  EXPECT_EQ(1, spectra[0]);
  EXPECT_EQ(9, spectra[8]);
  EXPECT_EQ(245768, spectra[245767]);
}
