#include <gmock/gmock.h>

#include "../include/NexusFileReader.h"

class NexusFileReaderTest : public ::testing::Test {};

extern std::string testDataPath;

TEST(NexusFileReaderTest, nexus_file_open_not_exist) {
  EXPECT_THROW(
      NexusFileReader(hdf5::file::open(testDataPath + "not_exist_file.nxs"), 0,
                      0, {0}),
      std::runtime_error);
}

TEST(NexusFileReaderTest, nexus_file_open_exists) {
  EXPECT_NO_THROW(NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0}));
}

TEST(NexusFileReaderTest, nexus_uncompressed_file_open_exists) {
  EXPECT_NO_THROW(NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test_reduced.hdf5"), 0, 0, {0}));
}

TEST(NexusFileReaderTest, nexus_read_file_size) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(104602171, fileReader.getFileSize());
}

TEST(NexusFileReaderTest, nexus_read_number_events) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(14258850, fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_fake_number_of_events) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  const int32_t numberOfFrames = 18131;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  EXPECT_EQ(numberOfFakeEventsPerPulse * numberOfFrames,
            fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_read_number_frames) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(18131, fileReader.getNumberOfFrames());
}

TEST(NexusFileReaderTest, get_detIds_first_frame) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<uint32_t> detIds;
  EXPECT_TRUE(fileReader.getEventDetIds(detIds, 0));
  EXPECT_EQ(99406, detIds[0]);
  EXPECT_EQ(87829, detIds[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<uint32_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_EQ(11660506, eventTofs[0]);
  EXPECT_EQ(46247304, eventTofs[150]);
}

TEST(NexusFileReaderTest,
     test_getting_fake_event_tofs_matches_specified_number_of_fake_events) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  std::vector<uint32_t> eventTofs;
  EXPECT_TRUE(fileReader.getEventTofs(eventTofs, 0));
  EXPECT_EQ(numberOfFakeEventsPerPulse, eventTofs.size());
}

TEST(NexusFileReaderTest,
     test_getting_fake_detids_matches_specified_number_of_fake_events) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  std::vector<uint32_t> detIDs;
  EXPECT_TRUE(fileReader.getEventDetIds(detIDs, 0));
  EXPECT_EQ(numberOfFakeEventsPerPulse, detIDs.size());
}

TEST(NexusFileReaderTest, get_detIds_too_high_frame_number) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<uint32_t> detIds;
  EXPECT_FALSE(fileReader.getEventDetIds(detIds, 3000000));
}

TEST(NexusFileReaderTest, get_event_tofs_too_high_frame_number) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<uint32_t> eventTofs;
  EXPECT_FALSE(fileReader.getEventTofs(eventTofs, 3000000));
}

TEST(NexusFileReaderTest, get_period_number) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(0, fileReader.getPeriodNumber());
}

TEST(NexusFileReaderTest, get_proton_charge) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(0));
  EXPECT_FLOAT_EQ(0.001105368, fileReader.getProtonCharge(7));
}

TEST(NexusFileReaderTest, get_number_of_events_in_frame) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(794, fileReader.getNumberOfEventsInFrame(0));
  EXPECT_EQ(781, fileReader.getNumberOfEventsInFrame(7));
}

TEST(NexusFileReaderTest,
     test_number_of_events_in_frame_matches_numer_of_fake_events_specified) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  EXPECT_EQ(numberOfFakeEventsPerPulse, fileReader.getNumberOfEventsInFrame(0));
  EXPECT_EQ(numberOfFakeEventsPerPulse, fileReader.getNumberOfEventsInFrame(7));
}

TEST(NexusFileReaderTest, get_frame_time) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(2940000057, fileReader.getFrameTime(0));
  EXPECT_EQ(3638999939, fileReader.getFrameTime(7));
}

TEST(NexusFileReaderTest, get_instrument_name) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ("SANS2D", fileReader.getInstrumentName());
}

TEST(NexusFileReaderTest, get_sEEvent_map) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  auto eventMap = fileReader.getSEEventMap();
  auto eventVector = eventMap[10];
  EXPECT_EQ(57, eventVector.size());
  EXPECT_EQ("Det_Temp_FLB", eventVector[0]->getName());
  EXPECT_EQ("Det_Temp_FRT", eventVector[3]->getName());
  EXPECT_EQ(1000000000, eventVector[3]->getTimestamp());
}

TEST(NexusFileReaderTest, get_number_of_periods) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test_reduced.hdf5"), 0, 0, {0});
  EXPECT_EQ(1, fileReader.getNumberOfPeriods());
}
