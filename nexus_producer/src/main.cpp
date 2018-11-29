#include "../../nexus_file_reader/include/NexusFileReader.h"
#include "KafkaEventPublisher.h"
#include "NexusPublisher.h"
#include "OptionalArgs.h"
#include <CLI/CLI.hpp>
#include <chrono>
#include <iostream>
#include <thread>

uint64_t getTimeNowNanosecondsFromEpoch() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  auto runStartTime = static_cast<uint64_t>(now_c) * 1000000000L;
  return runStartTime;
}

std::vector<int32_t> getDetectorNumbers(const OptionalArgs &settings) {
  auto detSpecMap = DetectorSpectrumMapData(settings.detSpecFilename);
  auto detectorNumbers = detSpecMap.getDetectors();
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
  App.add_option("-d,--det_spec_map", settings.detSpecFilename,
                 "Full path of the detector-spectrum map")
      ->check(CLI::ExistingFile)
      ->required();
  App.add_option("-b,--broker", settings.broker,
                 "Hostname or IP of Kafka broker")
      ->required();
  App.add_option("-i,--instrument", settings.instrumentName,
                 "Used as prefix for topic names")
      ->required();
  App.add_option("-m,--compression", settings.compression,
                 "Compression option for Kafka messages");
  App.add_option("-e,--fake_events_per_pulse", settings.fakeEventsPerPulse,
                 "Generates this number of fake events per pulse instead of "
                 "publishing real data from file");
  App.add_flag("-s,--slow", settings.slow,
               "Publish data at approx realistic rate (detected from file)");
  App.add_flag("-q,--quiet", settings.quietMode, "Less chatty on stdout");
  App.add_flag(
      "-z,--single_run", settings.singleRun,
      "Publish only a single run (otherwise repeats until interrupted)");
  App.set_config("-c,--config-file", "", "Read configuration from an ini file",
                 false);

  CLI11_PARSE(App, argc, argv);

  auto detectorNumbers = getDetectorNumbers(settings);
  auto runStartTime = getTimeNowNanosecondsFromEpoch();

  auto publisher = std::make_shared<KafkaEventPublisher>(settings.compression);
  auto fileReader = std::make_shared<NexusFileReader>(
      hdf5::file::open(settings.filename), runStartTime,
      settings.fakeEventsPerPulse, detectorNumbers);
  publisher->setUp(settings.broker, settings.instrumentName);
  int runNumber = 1;
  NexusPublisher streamer(publisher, fileReader, settings);

  // Publish the same data repeatedly, with incrementing run numbers
  if (settings.singleRun) {
    streamer.streamData(runNumber, settings.slow);
  } else {
    while (true) {
      streamer.streamData(runNumber, settings.slow);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      runNumber++;
    }
  }

  return 0;
}
