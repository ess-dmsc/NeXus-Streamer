#include <gmock/gmock.h>

#include "../include/NexusFileReader.h"

class NexusFileReaderTest : public ::testing::Test {};

TEST(NexusFileReaderTest, nexus_file_open_not_exist) {
  H5::Exception::dontPrint();
  extern std::string testDataPath;
  EXPECT_THROW(NexusFileReader(testDataPath + "not_exist_file.nxs"),
               H5::FileIException);
}

TEST(NexusFileReaderTest, nexus_file_open_exists) {
  extern std::string testDataPath;
  EXPECT_NO_THROW(NexusFileReader(testDataPath + "SANS_test.nxs"));
}

TEST(NexusFileReaderTest, nexus_uncompressed_file_open_exists) {
  extern std::string testDataPath;
  EXPECT_NO_THROW(NexusFileReader(testDataPath + "SANS_test_reduced.hdf5"));
}

TEST(NexusFileReaderTest, nexus_read_file_size) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(104602171, fileReader.getFileSize());
}

TEST(NexusFileReaderTest, nexus_read_number_events) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(14258850, fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_read_number_frames) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(18131, fileReader.getNumberOfFrames());
}

TEST(NexusFileReaderTest, get_detIds_first_frame) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  std::vector<uint32_t> detIds;
  EXPECT_TRUE(fileReader.getEventDetIds(detIds, 0));
  EXPECT_EQ(99406, detIds[0]);
  EXPECT_EQ(87829, detIds[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  std::vector<uint32_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_EQ(11660506, eventTofs[0]);
  EXPECT_EQ(46247304, eventTofs[150]);
}

TEST(NexusFileReaderTest, get_detIds_too_high_frame_number) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  std::vector<uint32_t> detIds;
  EXPECT_FALSE(fileReader.getEventDetIds(detIds, 3000000));
}

TEST(NexusFileReaderTest, get_event_tofs_too_high_frame_number) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  std::vector<uint32_t> eventTofs;
  EXPECT_FALSE(fileReader.getEventTofs(eventTofs, 3000000));
}

TEST(NexusFileReaderTest, get_period_number) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(0, fileReader.getPeriodNumber());
}

TEST(NexusFileReaderTest, get_proton_charge) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(0));
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(7));
}

TEST(NexusFileReaderTest, get_number_of_events_in_frame) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(794, fileReader.getNumberOfEventsInFrame(0));
  EXPECT_EQ(781, fileReader.getNumberOfEventsInFrame(7));
}

TEST(NexusFileReaderTest, get_frame_time) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(1460429934940000057, fileReader.getFrameTime(0));
  EXPECT_EQ(1460429935638999939, fileReader.getFrameTime(7));
}

TEST(NexusFileReaderTest, get_run_start_time) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(1460429932, fileReader.getRunStartTime());
}

TEST(NexusFileReaderTest, get_instrument_name) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ("SANS2D", fileReader.getInstrumentName());
}

TEST(NexusFileReaderTest, get_se_names) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  auto seNamesFromFile = fileReader.getNamesInGroup("/raw_data_1/selog");
  EXPECT_THAT(seNamesFromFile,
              ::testing::ElementsAre("Guide_Pressure", "Rear_Det_X",
                                     "SECI_OUT_OF_RANGE_BLOCK", "Sample",
                                     "TEMP1", "fake_int", "fake_long"));
}

TEST(NexusFileReaderTest, get_frame_parts_per_frame) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  auto framePartsPerFrame = fileReader.getFramePartsPerFrame(200);
  EXPECT_EQ(4, framePartsPerFrame[0]);
}

TEST(NexusFileReaderTest, get_1D_dataset_float) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  auto valueVector = fileReader.get1DDataset<float>(
      H5::PredType::NATIVE_FLOAT,
      "/raw_data_1/selog/Guide_Pressure/value_log/value");
  EXPECT_FLOAT_EQ(0.18, valueVector[4]);
}

TEST(NexusFileReaderTest, get_1D_dataset_string) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  auto eventVector = fileReader.get1DStringDataset(
      "/raw_data_1/selog/SECI_OUT_OF_RANGE_BLOCK/value_log/value");
  EXPECT_EQ("Fast_Shutter", eventVector[0].substr(0, 12));
}

TEST(NexusFileReaderTest, get_sEEvent_map) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  auto eventMap = fileReader.getSEEventMap();
  auto eventVector = eventMap[10];
  EXPECT_EQ(4, eventVector.size());
  EXPECT_EQ("Guide_Pressure", eventVector[0]->getName());
  EXPECT_EQ("TEMP1", eventVector[3]->getName());
  EXPECT_EQ(829277933, eventVector[3]->getTimestamp());
}

TEST(NexusFileReaderTest, get_number_of_periods) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test_reduced.hdf5");
  EXPECT_EQ(1, fileReader.getNumberOfPeriods());
}
