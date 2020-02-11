#include <6s4t_run_stop_generated.h>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <pl72_run_start_generated.h>

#include "DetectorSpectrumMapData.h"
#include "RunData.h"

class RunDataTest : public ::testing::Test {};

TEST(RunDataTest, set_and_get_start_time) {
  auto runData = RunData();
  runData.setStartTimeFromString("2016-08-11T08:50:18");
  EXPECT_EQ(1470905418000000000, runData.startTime);

  runData.setStartTimeInSeconds(1470905418);
  EXPECT_EQ(1470905418000000000, runData.startTime);
}

TEST(RunDataTest, set_and_get_stop_time) {
  auto runData = RunData();
  runData.setStopTimeFromString("2016-08-13T13:32:09");
  EXPECT_EQ(1471095129000000000, runData.stopTime);
}

TEST(RunDataTest, get_run_info_as_string) {
  auto runData = RunData();
  runData.instrumentName = "SANS2D";
  runData.runID = "42";
  runData.setStartTimeFromString("2016-08-11T08:50:18");

  EXPECT_EQ("Run ID: 42, Instrument name: SANS2D, Start time: "
            "2016-08-11T08:50:18",
            fmt::format("{}", runData));
}

TEST(RunDataTest, encode_and_decode_run_start) {
  auto inputRunData = RunData();
  inputRunData.setStartTimeFromString("2016-08-11T08:50:18");
  inputRunData.setStopTimeFromString("2016-08-11T08:50:18");
  inputRunData.runID = "42";
  inputRunData.instrumentName = "SANS2D";
  inputRunData.nexusStructure = "{}";
  inputRunData.jobID = "job42";
  inputRunData.serviceID = "service42";
  inputRunData.numberOfPeriods = 3;
  inputRunData.broker = "localhost:9092";
  inputRunData.filename = "testfile.nxs";

  extern std::string testDataPath;
  auto detSpecMap = nonstd::optional<DetectorSpectrumMapData>(
      DetectorSpectrumMapData(testDataPath + "spectrum_gastubes_01.dat"));

  auto runStartMessage = serialiseRunStartMessage(inputRunData, detSpecMap);
  auto outputRunData = deserialiseRunStartMessage(
      reinterpret_cast<const uint8_t *>(runStartMessage.data()));

  EXPECT_EQ(outputRunData.startTime, inputRunData.startTime);
  EXPECT_EQ(outputRunData.stopTime, inputRunData.stopTime);
  EXPECT_EQ(outputRunData.runID, inputRunData.runID);
  EXPECT_EQ(outputRunData.instrumentName, inputRunData.instrumentName);
  EXPECT_EQ(outputRunData.nexusStructure, inputRunData.nexusStructure);
  EXPECT_EQ(outputRunData.jobID, inputRunData.jobID);
  EXPECT_EQ(outputRunData.serviceID, inputRunData.serviceID);
  EXPECT_EQ(outputRunData.numberOfPeriods, inputRunData.numberOfPeriods);
  EXPECT_EQ(outputRunData.broker, inputRunData.broker);
  EXPECT_EQ(outputRunData.filename, inputRunData.filename);

  // Check detector spectrum map data were successfully added to message
  auto runStartData =
      GetRunStart(reinterpret_cast<const uint8_t *>(runStartMessage.data()));
  auto receivedMapData =
      DetectorSpectrumMapData(runStartData->detector_spectrum_map());

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

TEST(RunDataTest, encode_and_decode_run_stop) {
  auto inputRunData = RunData();
  inputRunData.setStopTimeFromString("2016-08-11T08:50:18");
  inputRunData.runID = "42";
  inputRunData.jobID = "job42";
  inputRunData.serviceID = "service42";

  auto runStopMessage = serialiseRunStopMessage(inputRunData);
  auto outputRunData = deserialiseRunStopMessage(
      reinterpret_cast<const uint8_t *>(runStopMessage.data()));

  EXPECT_EQ(outputRunData.stopTime, inputRunData.stopTime);
  EXPECT_EQ(outputRunData.runID, inputRunData.runID);
  EXPECT_EQ(outputRunData.jobID, inputRunData.jobID);
  EXPECT_EQ(outputRunData.serviceID, inputRunData.serviceID);
}

TEST(RunDataTest, check_stop_message_includes_file_identifier) {
  auto runData = RunData();
  auto runMessage = serialiseRunStopMessage(runData);
  auto runIdentifier = RunStopIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(runMessage.data()), runIdentifier));
}

TEST(RunDataTest, check_start_message_includes_file_identifier) {
  auto runData = RunData();
  nonstd::optional<DetectorSpectrumMapData> optionalDetSpecMap =
      nonstd::nullopt;
  auto runMessage = serialiseRunStartMessage(runData, optionalDetSpecMap);
  auto runIdentifier = RunStartIdentifier();
  EXPECT_TRUE(flatbuffers::BufferHasIdentifier(
      reinterpret_cast<const uint8_t *>(runMessage.data()), runIdentifier));
}
