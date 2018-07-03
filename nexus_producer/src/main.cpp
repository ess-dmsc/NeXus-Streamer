#include <CLI/CLI.hpp>
#include <chrono>
#include <iostream>
#include <thread>

#include "KafkaEventPublisher.h"
#include "NexusPublisher.h"

int main(int argc, char **argv) {

  CLI::App App{"Stream neutron detection event and sample environment data "
               "from a NeXus file into Kafka"};

  std::string filename;
  std::string detSpecFilename;
  std::string broker;
  std::string instrumentName = "test";
  std::string compression;
  bool slow = false;
  bool quietMode = false;
  bool singleRun = false;
  int32_t fakeEventsPerPulse = 0;

  App.add_option("--filename", filename, "Full path of the NeXus file")
      ->required();
  App.add_option("--detspecmap", detSpecFilename,
                 "Full path of the detector-spectrum map")
      ->required();
  App.add_option("--broker", broker, "Hostname or IP of Kafka broker")
      ->required();
  App.add_option("--instrument", instrumentName,
                 "Used as prefix for topic names");
  App.add_option("--compression", compression,
                 "Compression option for Kafka messages");
  App.add_option("--fakeeventsperpulse", fakeEventsPerPulse,
                 "Generates this number of fake events per pulse instead of "
                 "publishing real data from file");
  App.add_flag("--slow", slow,
               "Publish data at approx realistic rate (10 pulses per second)");
  App.add_flag("--quiet", quietMode, "Less chatty on stdout");
  App.add_flag(
      "--singlerun", singleRun,
      "Publish only a single run (otherwise repeats until interrupted)");

  CLI11_PARSE(App, argc, argv);

  auto publisher = std::make_shared<KafkaEventPublisher>(compression);
  int runNumber = 1;
  NexusPublisher streamer(publisher, broker, instrumentName, filename,
                          detSpecFilename, quietMode, fakeEventsPerPulse);

  // Publish the same data repeatedly, with incrementing run numbers
  if (singleRun) {
    streamer.streamData(runNumber, slow);
  } else {
    while (true) {
      streamer.streamData(runNumber, slow);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      runNumber++;
    }
  }

  return 0;
}
