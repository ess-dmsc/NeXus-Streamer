#include <gmock/gmock.h>

#include "../include/NexusFileReader.h"

class NexusFileReaderTest : public ::testing::Test {};

extern std::string testDataPath;

TEST(NexusFileReaderTest, nexus_file_open_not_exist) {
  H5::Exception::dontPrint();
  EXPECT_THROW(NexusFileReader(testDataPath + "not_exist_file.nxs", 0, 0, {0}),
               H5::FileIException);
}

TEST(NexusFileReaderTest, nexus_file_open_exists) {
  EXPECT_NO_THROW(NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0}));
}

TEST(NexusFileReaderTest, nexus_uncompressed_file_open_exists) {
  EXPECT_NO_THROW(
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0}));
}

TEST(NexusFileReaderTest, nexus_read_file_size) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(104602171, fileReader.getFileSize());
}

TEST(NexusFileReaderTest, nexus_read_number_events) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(14258850, fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_read_number_frames) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(18131, fileReader.getNumberOfFrames());
}

TEST(NexusFileReaderTest, get_detIds_first_frame) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  std::vector<uint32_t> detIds;
  EXPECT_TRUE(fileReader.getEventDetIds(detIds, 0));
  EXPECT_EQ(99406, detIds[0]);
  EXPECT_EQ(87829, detIds[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  std::vector<uint32_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_EQ(11660506, eventTofs[0]);
  EXPECT_EQ(46247304, eventTofs[150]);
}

TEST(NexusFileReaderTest, get_detIds_too_high_frame_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  std::vector<uint32_t> detIds;
  EXPECT_FALSE(fileReader.getEventDetIds(detIds, 3000000));
}

TEST(NexusFileReaderTest, get_event_tofs_too_high_frame_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  std::vector<uint32_t> eventTofs;
  EXPECT_FALSE(fileReader.getEventTofs(eventTofs, 3000000));
}

TEST(NexusFileReaderTest, get_period_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(0, fileReader.getPeriodNumber());
}

TEST(NexusFileReaderTest, get_proton_charge) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(0));
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(7));
}

TEST(NexusFileReaderTest, get_number_of_events_in_frame) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(794, fileReader.getNumberOfEventsInFrame(0));
  EXPECT_EQ(781, fileReader.getNumberOfEventsInFrame(7));
}

TEST(NexusFileReaderTest, get_frame_time) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ(1460429934940000057, fileReader.getFrameTime(0));
  EXPECT_EQ(1460429935638999939, fileReader.getFrameTime(7));
}

TEST(NexusFileReaderTest, get_instrument_name) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0, 0, {0});
  EXPECT_EQ("SANS2D", fileReader.getInstrumentName());
}

TEST(NexusFileReaderTest, get_se_names) {
  auto fileReader =
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0});
  auto seNamesFromFile = fileReader.getNamesInGroup("/raw_data_1/selog");
  EXPECT_THAT(seNamesFromFile,
              ::testing::ElementsAre("Guide_Pressure", "Rear_Det_X",
                                     "SECI_OUT_OF_RANGE_BLOCK", "Sample",
                                     "TEMP1", "fake_int", "fake_long"));
}

TEST(NexusFileReaderTest, get_1D_dataset_float) {
  auto fileReader =
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0});
  auto valueVector = fileReader.get1DDataset<float>(
      H5::PredType::NATIVE_FLOAT,
      "/raw_data_1/selog/Guide_Pressure/value_log/value");
  EXPECT_FLOAT_EQ(0.18, valueVector[4]);
}

TEST(NexusFileReaderTest, get_1D_dataset_string) {
  auto fileReader =
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0});
  auto eventVector = fileReader.get1DStringDataset(
      "/raw_data_1/selog/SECI_OUT_OF_RANGE_BLOCK/value_log/value");
  EXPECT_EQ("Fast_Shutter", eventVector[0].substr(0, 12));
}

TEST(NexusFileReaderTest, get_sEEvent_map) {
  auto fileReader =
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0});
  auto eventMap = fileReader.getSEEventMap();
  auto eventVector = eventMap[10];
  EXPECT_EQ(4, eventVector.size());
  EXPECT_EQ("Guide_Pressure", eventVector[0]->getName());
  EXPECT_EQ("TEMP1", eventVector[3]->getName());
  EXPECT_EQ(1000000000, eventVector[3]->getTimestamp());
}

TEST(NexusFileReaderTest, get_number_of_periods) {
  auto fileReader =
      NexusFileReader(testDataPath + "SANS_test_reduced.hdf5", 0, 0, {0});
  EXPECT_EQ(1, fileReader.getNumberOfPeriods());
}
