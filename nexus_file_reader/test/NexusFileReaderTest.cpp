#include <gmock/gmock.h>

#include "../include/NexusFileReader.h"

class NexusFileReaderTest : public ::testing::Test {};

extern std::string testDataPath;

TEST(NexusFileReaderTest, nexus_file_open_not_exist) {
  EXPECT_THROW(NexusFileReader(testDataPath + "not_exist_file.nxs", 0),
               std::exception);
}

TEST(NexusFileReaderTest, nexus_file_open_exists) {
  std::cout << testDataPath + "SANS_test.nxs" << std::endl;
  EXPECT_NO_THROW(NexusFileReader(testDataPath + "SANS_test.nxs", 0));
}

TEST(NexusFileReaderTest, nexus_read_file_size) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(104602171, fileReader.getFileSize());
}

TEST(NexusFileReaderTest, nexus_read_number_events) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(14258850, fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_read_number_frames) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(18131, fileReader.getNumberOfFrames());
}

TEST(NexusFileReaderTest, get_detIds_first_frame) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  std::vector<uint32_t> detIds;
  EXPECT_TRUE(fileReader.getEventDetIds(detIds, 0));
  EXPECT_EQ(99406, detIds[0]);
  EXPECT_EQ(87829, detIds[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  std::vector<uint32_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_EQ(11660506, eventTofs[0]);
  EXPECT_EQ(46247304, eventTofs[150]);
}

TEST(NexusFileReaderTest, get_detIds_too_high_frame_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  std::vector<uint32_t> detIds;
  EXPECT_FALSE(fileReader.getEventDetIds(detIds, 3000000));
}

TEST(NexusFileReaderTest, get_event_tofs_too_high_frame_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  std::vector<uint32_t> eventTofs;
  EXPECT_FALSE(fileReader.getEventTofs(eventTofs, 3000000));
}

TEST(NexusFileReaderTest, get_period_number) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(0, fileReader.getPeriodNumber());
}

TEST(NexusFileReaderTest, get_proton_charge) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(0));
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(7));
}

TEST(NexusFileReaderTest, get_number_of_events_in_frame) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(794, fileReader.getNumberOfEventsInFrame(0));
  EXPECT_EQ(781, fileReader.getNumberOfEventsInFrame(7));
}

TEST(NexusFileReaderTest, get_frame_time) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(1460429934940000057, fileReader.getFrameTime(0));
  EXPECT_EQ(1460429935638999939, fileReader.getFrameTime(7));
}

TEST(NexusFileReaderTest, get_instrument_name) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ("SANS2D", fileReader.getInstrumentName());
}

TEST(NexusFileReaderTest, get_sEEvent_map) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  auto eventMap = fileReader.getSEEventMap();
  auto eventVector = eventMap[10];
  EXPECT_EQ(57, eventVector.size());
  EXPECT_EQ("Det_Temp_FLB", eventVector[0]->getName());
  EXPECT_EQ("Det_Temp_FRT", eventVector[3]->getName());
  EXPECT_EQ(1000000000, eventVector[3]->getTimestamp());
}

TEST(NexusFileReaderTest, get_number_of_periods) {
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs", 0);
  EXPECT_EQ(1, fileReader.getNumberOfPeriods());
}
