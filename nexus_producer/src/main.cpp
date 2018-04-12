#ifdef _MSC_VER
#include "win32/wingetopt.h"
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <chrono>
#include <iostream>
#include <thread>

#include "KafkaEventPublisher.h"
#include "NexusPublisher.h"

int main(int argc, char **argv) {

  int opt;
  std::string filename;
  std::string detSpecFilename;
  std::string broker;
  std::string instrumentName = "test";
  std::string compression;
  bool slow = false;
  bool quietMode = false;
  bool singleRun = false;

  while ((opt = getopt(argc, argv, "f:d:b:i:c:sqz")) != -1) {
    switch (opt) {

    case 'f':
      filename = optarg;
      break;

    case 'd':
      detSpecFilename = optarg;
      break;

    case 'b':
      broker = optarg;
      break;

    case 'i':
      instrumentName = optarg;
      break;

    case 'c':
      compression = optarg;
      break;

    case 's':
      slow = true;
      break;

    case 'q':
      quietMode = true;
      break;

    case 'z':
      singleRun = true;
      break;

    default:
      goto usage;
    }
  }

  if (filename.empty() || detSpecFilename.empty() || broker.empty()) {
  usage:
    std::cerr
        << "Usage:\n"
        << argv[0]
        << " -f <filepath>    Full file path of nexus file to stream\n"
           "-d <det_spec_map_filepath>    Full file path of file defining the "
           "det-spec mapping\n"
           "-b <host>    Broker IP address or hostname\n"
           "[-i <instrument_name>]    Used as prefix for topic names\n"
           "[-s]    Slow mode, publishes data at approx realistic rate of 10 "
           "frames per second\n"
           "[-q]    Quiet mode, makes publisher less chatty on stdout\n"
           "[-z]    Produce only a single run (otherwise repeats until "
           "interrupted)"
           "\n";
    exit(1);
  }

  auto publisher = std::make_shared<KafkaEventPublisher>(compression);
  int runNumber = 1;
  NexusPublisher streamer(publisher, broker, instrumentName, filename,
                          detSpecFilename, quietMode);

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
