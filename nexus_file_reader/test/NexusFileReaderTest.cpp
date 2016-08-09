#include <gtest/gtest.h>

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
  EXPECT_FLOAT_EQ(99406, detIds[0]);
  EXPECT_FLOAT_EQ(87829, detIds[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  std::vector<uint64_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_FLOAT_EQ(11660505, eventTofs[0]);
  EXPECT_FLOAT_EQ(46247304, eventTofs[150]);
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
  std::vector<uint64_t> eventTofs;
  EXPECT_FALSE(fileReader.getEventTofs(eventTofs, 3000000));
}

TEST(NexusFileReaderTest, get_period_number) {
  extern std::string testDataPath;
  auto fileReader = NexusFileReader(testDataPath + "SANS_test.nxs");
  EXPECT_EQ(1, fileReader.getPeriodNumber());
}