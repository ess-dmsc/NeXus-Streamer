#include <CLI/CLI.hpp>
#include <chrono>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

#include "../../core/include/OptionalArgs.h"
#include "../../nexus_file_reader/include/NexusFileReader.h"
#include "../../serialisation/include/DetectorSpectrumMapData.h"
#include "JSONDescriptionLoader.h"
#include "KafkaPublisher.h"
#include "NexusPublisher.h"

uint64_t getTimeNowNanosecondsFromEpoch() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  auto runStartTime = static_cast<uint64_t>(now_c) * 1000000000L;
  return runStartTime;
}

std::vector<int32_t> getDetectorNumbers(const OptionalArgs &settings) {
  std::vector<int32_t> detectorNumbers;
  if (settings.minMaxDetectorNums.first > 0 &&
      settings.minMaxDetectorNums.second > 0) {
    // Allocate space in vector equal to the difference between the minimum and
    // maximum detector number. Then use iota to fill with sequential values
    // until the vector is full.
    detectorNumbers =
        std::vector<int32_t>(settings.minMaxDetectorNums.second -
                             settings.minMaxDetectorNums.first + 1);
    std::iota(detectorNumbers.begin(), detectorNumbers.end(),
              settings.minMaxDetectorNums.first);
  } else {
    auto detSpecMap = DetectorSpectrumMapData(settings.detSpecFilename);
    detectorNumbers = detSpecMap.getDetectors();
  }
  return detectorNumbers;
}

int main(int argc, char **argv) {

  CLI::App App{"Stream neutron detection event and sample environment data "
               "from a NeXus file into Kafka"};

  OptionalArgs settings;

  App.add_option("-f,--filename", settings.filename,
                 "Full path of the NeXus file")
      ->check(CLI::ExistingFile)
      ->required();
  App.add_option("-d,--det-spec-map", settings.detSpecFilename,
                 "Full path of the detector-spectrum map")
      ->check(CLI::ExistingFile);
  App.add_option("-b,--broker", settings.broker,
                 "Hostname or IP of Kafka broker")
      ->required();
  App.add_option("-i,--instrument", settings.instrumentName,
                 "Used as prefix for topic names")
      ->required();
  App.add_option("-m,--compression", settings.compression,
                 "Compression option for Kafka messages");
  App.add_option("-e,--fake-events-per-pulse", settings.fakeEventsPerPulse,
                 "Generates this number of fake events per pulse per "
                 "NXevent_data instead of "
                 "publishing real data from file");
  App.add_option("--histogram-update-period", settings.histogramUpdatePeriodMs,
                 "Publish a histogram data message with this period (in "
                 "integer milliseconds) default 0 means do not stream "
                 "histograms");
  App.add_option("--json-description", settings.jsonDescription,
                 "Optionally provide the path to a file containing a json "
                 "description of the NeXus file, "
                 "this should match the contents of the nexus_structure field "
                 "described here: "
                 "https://github.com/ess-dmsc/kafka-to-nexus/blob/master/"
                 "documentation/commands.md")
      ->check(CLI::ExistingFile);
  App.add_option(
         "-x,--disable-map",
         [&settings](CLI::results_t Results) {
           auto min = std::stoi(Results.at(0));
           auto max = std::stoi(Results.at(1));
           if (min >= max)
             throw std::runtime_error(
                 "given MIN detector number is larger than or equal to MAX");
           settings.minMaxDetectorNums = std::make_pair(min, max);
           return true;
         },
         "Use MIN and MAX detector numbers in inclusive range instead of using "
         "a det-spec map file")
      ->type_size(2)
      ->type_name("INT INT");
  App.add_flag("-s,--slow", settings.slow,
               "Publish data at approx realistic rate (detected from file)");
  App.add_flag("-q,--quiet", settings.quietMode, "Less chatty on stdout");
  App.add_flag(
      "-z,--single-run", settings.singleRun,
      "Publish only a single run (otherwise repeats until interrupted)");
  App.set_config("-c,--config-file", "", "Read configuration from an ini file",
                 false);

  CLI11_PARSE(App, argc, argv);
  auto Logger = spdlog::stderr_color_mt("LOG");
  Logger->debug("Application launched");

  auto detectorNumbers = getDetectorNumbers(settings);
  auto runStartTime = getTimeNowNanosecondsFromEpoch();

  auto publisher = std::make_shared<KafkaPublisher>(settings.compression);
  auto fileReader = std::make_shared<NexusFileReader>(
      hdf5::file::open(settings.filename), runStartTime,
      settings.fakeEventsPerPulse, detectorNumbers, settings);
  publisher->setUp(settings.broker, settings.instrumentName);
  int runNumber = 1;
  NexusPublisher streamer(publisher, fileReader, settings);

  // Publish the same data repeatedly, with incrementing run numbers
  std::string jsonDescription =
      JSONDescriptionLoader::loadJsonFromFile(settings.jsonDescription);
  if (jsonDescription.empty()) {
    Logger->warn("No JSON description of the NeXus file provided");
  }
  JSONDescriptionLoader::updateTopicNames(jsonDescription,
                                          settings.instrumentName);
  if (settings.singleRun) {
    streamer.streamData(runNumber, settings, jsonDescription);
  } else {
    while (true) {
      streamer.streamData(runNumber, settings, jsonDescription);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      runNumber++;
    }
  }

  return 0;
}
