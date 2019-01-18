#include <gmock/gmock.h>

#include "../include/NexusFileReader.h"
#include "HDF5FileTestHelpers.h"

class NexusFileReaderTest : public ::testing::Test {};

extern std::string testDataPath;

using HDF5FileTestHelpers::createInMemoryTestFile;
using HDF5FileTestHelpers::createInMemoryTestFileWithEventData;

TEST(NexusFileReaderTest, error_thrown_for_non_existent_file) {
  EXPECT_THROW(
      NexusFileReader(hdf5::file::open(testDataPath + "not_exist_file.nxs"), 0,
                      0, {0}),
      std::runtime_error);
}

TEST(NexusFileReaderTest, error_thrown_for_file_with_no_NXentry_group) {
  EXPECT_THROW(NexusFileReader(createInMemoryTestFile("fileWithNoNXentry.nxs"),
                               0, 0, {0}),
               std::runtime_error);
}

TEST(NexusFileReaderTest,
     error_thrown_for_file_with_NXentry_but_no_event_data_group) {
  auto file = createInMemoryTestFile("fileWithNoEventData.nxs");
  HDF5FileTestHelpers::addNXentryToFile(file);

  EXPECT_THROW(NexusFileReader(file, 0, 0, {0}), std::runtime_error);
}

TEST(
    NexusFileReaderTest,
    error_thrown_for_file_with_NXentry_and_event_data_group_but_no_num_of_frames) {
  auto file = createInMemoryTestFile("fileWithNoGoodFrames.nxs");
  HDF5FileTestHelpers::addNXentryToFile(file);
  HDF5FileTestHelpers::addNXeventDataToFile(file);

  EXPECT_THROW(NexusFileReader(file, 0, 0, {0}), std::runtime_error);
}

TEST(NexusFileReaderTest,
     no_error_thrown_when_file_exists_with_requisite_groups) {
  auto file =
      createInMemoryTestFileWithEventData("fileWithRequisiteGroups.nxs");

  EXPECT_NO_THROW(NexusFileReader(file, 0, 0, {0}));
}

TEST(NexusFileReaderTest, expect_empty_map_if_no_selog_group_present) {
  auto file =
      createInMemoryTestFileWithEventData("fileWithRequisiteGroups.nxs");

  auto fileReader = NexusFileReader(file, 0, 0, {0});
  auto sEMap = fileReader.getSEEventMap();
  EXPECT_EQ(sEMap.size(), 0);
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
  const int32_t numberOfFrames = 18132;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  EXPECT_EQ(numberOfFakeEventsPerPulse * numberOfFrames,
            fileReader.getTotalEventCount());
}

TEST(NexusFileReaderTest, nexus_read_number_frames) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(18132, fileReader.getNumberOfFrames());
}

TEST(NexusFileReaderTest, get_detIds_first_frame) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<EventDataFrame> eventData;
  EXPECT_TRUE(fileReader.getEventData(eventData, 0));
  EXPECT_EQ(99406, eventData[0].detectorIDs[0]);
  EXPECT_EQ(87829, eventData[0].detectorIDs[150]);
}

TEST(NexusFileReaderTest, get_event_tofs) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<EventDataFrame> eventData;
  EXPECT_TRUE(fileReader.getEventData(eventData, 0));
  EXPECT_EQ(11660506, eventData[0].timeOfFlights[0]);
  EXPECT_EQ(46247304, eventData[0].timeOfFlights[150]);
}

TEST(NexusFileReaderTest,
     test_getting_fake_event_tofs_matches_specified_number_of_fake_events) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  std::vector<EventDataFrame> eventData;
  EXPECT_TRUE(fileReader.getEventData(eventData, 0));
  EXPECT_EQ(numberOfFakeEventsPerPulse, eventData[0].timeOfFlights.size());
}

TEST(NexusFileReaderTest,
     test_getting_fake_detids_matches_specified_number_of_fake_events) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  std::vector<EventDataFrame> eventData;
  EXPECT_TRUE(fileReader.getEventData(eventData, 0));
  EXPECT_EQ(numberOfFakeEventsPerPulse, eventData[0].detectorIDs.size());
}

TEST(NexusFileReaderTest, get_eventData_too_high_frame_number) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  std::vector<EventDataFrame> eventData;
  EXPECT_FALSE(fileReader.getEventData(eventData, 3000000));
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
  EXPECT_EQ(794, fileReader.getNumberOfEventsInFrame(0, 0));
  EXPECT_EQ(781, fileReader.getNumberOfEventsInFrame(7, 0));
}

TEST(NexusFileReaderTest,
     test_number_of_events_in_frame_matches_numer_of_fake_events_specified) {
  const int32_t numberOfFakeEventsPerPulse = 10;
  auto fileReader =
      NexusFileReader(hdf5::file::open(testDataPath + "SANS_test.nxs"), 0,
                      numberOfFakeEventsPerPulse, {0});
  EXPECT_EQ(numberOfFakeEventsPerPulse,
            fileReader.getNumberOfEventsInFrame(0, 0));
  EXPECT_EQ(numberOfFakeEventsPerPulse,
            fileReader.getNumberOfEventsInFrame(7, 0));
}

TEST(NexusFileReaderTest, get_frame_time) {
  auto fileReader = NexusFileReader(
      hdf5::file::open(testDataPath + "SANS_test.nxs"), 0, 0, {0});
  EXPECT_EQ(2940000057, fileReader.getFrameTime(0));
  EXPECT_EQ(3638999939, fileReader.getFrameTime(7));
}

TEST(NexusFileReaderTest,
     get_relative_frame_time_returns_frame_time_from_file_in_milliseconds) {
  int64_t relativeFrameTimeInSeconds = 42;

  auto file = createInMemoryTestFile("fileWithEventData");
  HDF5FileTestHelpers::addNXentryToFile(file);
  HDF5FileTestHelpers::addNXeventDataToFile(file);
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(
      file, {relativeFrameTimeInSeconds}, {2}, {3}, {4});

  auto fileReader = NexusFileReader(file, 0, 0, {0});
  auto relativeFrameTimeInMilliseconds = relativeFrameTimeInSeconds * 1000;
  EXPECT_EQ(relativeFrameTimeInMilliseconds,
            fileReader.getRelativeFrameTimeMilliseconds(0));
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

TEST(NexusFileReaderTest, file_is_detected_as_from_isis_by_groups_present) {
  auto file = createInMemoryTestFile("fileWithISISGroups");
  // "raw_data_1" is the expected entry group name for an ISIS file
  HDF5FileTestHelpers::addNXentryToFile(file, "raw_data_1");
  HDF5FileTestHelpers::addNXeventDataToFile(file, "raw_data_1");
  HDF5FileTestHelpers::addVMSCompatGroupToFile(file);
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(file, "raw_data_1");
  auto fileReader = NexusFileReader(file, 0, 0, {0});
  EXPECT_TRUE(fileReader.isISISFile());
}

TEST(NexusFileReaderTest,
     if_no_vmscompat_group_present_then_file_not_detected_as_from_isis) {
  auto file = createInMemoryTestFile("fileWithISISEntryGroup");
  // "raw_data_1" is the expected entry group name for an ISIS file
  HDF5FileTestHelpers::addNXentryToFile(file, "raw_data_1");
  HDF5FileTestHelpers::addNXeventDataToFile(file, "raw_data_1");
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(file, "raw_data_1");
  auto fileReader = NexusFileReader(file, 0, 0, {0});
  // No "isis_vms_compat" group in file, so should assume not an ISIS file
  EXPECT_FALSE(fileReader.isISISFile());
}

TEST(NexusFileReaderTest,
     multiple_event_data_per_frame_if_multiple_event_data_groups_in_file) {
  auto file = createInMemoryTestFile("fileWithMultipleEventDataGroups");
  HDF5FileTestHelpers::addNXentryToFile(file, "entry");

  // Add two event data groups
  HDF5FileTestHelpers::addNXeventDataToFile(file, "entry", "detector_1_events");
  const std::vector<int64_t> det_1_event_time_zero{0};
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(
      file, {1}, {2}, {0}, {4}, "entry", "detector_1_events");
  HDF5FileTestHelpers::addNXeventDataToFile(file, "entry", "detector_2_events");
  const std::vector<int64_t> det_2_event_time_zero{1};
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(
      file, {1}, {2}, {0}, {4}, "entry", "detector_2_events");

  auto fileReader = NexusFileReader(file, 0, 0, {0});
  std::vector<EventDataFrame> eventData;
  fileReader.getEventData(eventData, 0);
  ASSERT_EQ(eventData.size(), 2) << "Expected two event data structs as there "
                                    "are two event groups in the input file";
}

TEST(NexusFileReaderTest,
     only_one_event_group_published_if_groups_have_inconsistent_pulse_data) {
  // Dealing with inconsistent pulse data between NXevent_data is complex and
  // may not be required, so it is not implemented
  // A warning log should be generated and the reader should proceed with
  // publishing data from one of the groups

  auto file = createInMemoryTestFile("fileWithInconsistentPulseData");
  HDF5FileTestHelpers::addNXentryToFile(file, "entry");

  // Add two event data groups with differing event_time_zero datasets
  HDF5FileTestHelpers::addNXeventDataToFile(file, "entry", "detector_1_events");
  const std::vector<int64_t> det_1_event_time_zero{0};
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(
      file, det_1_event_time_zero, {2}, {0}, {4}, "entry", "detector_1_events");
  HDF5FileTestHelpers::addNXeventDataToFile(file, "entry", "detector_2_events");
  const std::vector<int64_t> det_2_event_time_zero{1};
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(
      file, det_2_event_time_zero, {2}, {0}, {4}, "entry", "detector_2_events");

  auto fileReader = NexusFileReader(file, 0, 0, {0});
  std::vector<EventDataFrame> eventData;
  fileReader.getEventData(eventData, 0);
  ASSERT_EQ(eventData.size(), 1)
      << "Expected an event message from only one event data group as the "
         "multiple groups have inconsistent pulse times, which is not yet "
         "supported";
}
