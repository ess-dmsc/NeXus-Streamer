#include <gtest/gtest.h>

#include "../include/DetectorSpectrumMapData.h"

class DetectorSpectrumMapDataTest : public ::testing::Test {};

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_file) {
  extern std::string testDataPath;
  EXPECT_NO_THROW(
      DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat"));
}

TEST(DetectorSpectrumMapDataTest, non_existent_detector_spectrum_file) {
  extern std::string testDataPath;
  EXPECT_THROW(DetectorSpectrumMapData(testDataPath + "NON_EXISTENT_FILE.dat"),
               std::runtime_error);
}

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_detectors) {
  extern std::string testDataPath;
  auto detSpecMap =
      DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");
  EXPECT_EQ(122888, detSpecMap.getNumberOfEntries());

  auto detectors = detSpecMap.getDetectors();
  EXPECT_EQ(1, detectors[0]);
  EXPECT_EQ(1100000, detectors[8]);
  EXPECT_EQ(2523511, detectors[122887]);
}

TEST(DetectorSpectrumMapDataTest, read_detector_spectrum_map_spectra) {
  extern std::string testDataPath;
  auto detSpecMap =
      DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");
  EXPECT_EQ(122888, detSpecMap.getNumberOfEntries());

  auto spectra = detSpecMap.getSpectra();
  EXPECT_EQ(1, spectra[0]);
  EXPECT_EQ(9, spectra[8]);
  EXPECT_EQ(122888, spectra[122887]);
}

TEST(DetectorSpectrumMapDataTest, create_message_buffer) {
  extern std::string testDataPath;
  auto detSpecMap =
      DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat");

  std::string rawbuf;
  EXPECT_NO_THROW(detSpecMap.getBufferPointer(rawbuf));

  auto receivedMapData = DetectorSpectrumMapData();
  EXPECT_NO_THROW(receivedMapData.decodeMessage(
      reinterpret_cast<const uint8_t *>(rawbuf.c_str())));
  EXPECT_EQ(122888, receivedMapData.getNumberOfEntries());

  auto detectors = receivedMapData.getDetectors();
  EXPECT_EQ(1, detectors[0]);
  EXPECT_EQ(1100000, detectors[8]);
  EXPECT_EQ(2523511, detectors[122887]);

  auto spectra = receivedMapData.getSpectra();
  EXPECT_EQ(1, spectra[0]);
  EXPECT_EQ(9, spectra[8]);
  EXPECT_EQ(122888, spectra[122887]);
}
