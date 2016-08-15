#include <gtest/gtest.h>

#include "../include/DetectorSpectrumMapData.h"

class DetectorSpectrumMapDataTest : public ::testing::Test {};

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_file) {
  extern std::string testDataPath;
  EXPECT_NO_THROW(DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat"));
}

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_data) {
  extern std::string testDataPath;
  auto detSpecMap = DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");
  EXPECT_EQ(245768, detSpecMap.getNumberOfEntries());
}
