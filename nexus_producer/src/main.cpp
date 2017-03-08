#ifdef _MSC_VER
#include "win32/wingetopt.h"
#elif _AIX
#include <unistd.h>
#else
#include <getopt.h>
#endif
#include <iostream>

#include "KafkaEventPublisher.h"
#include "NexusPublisher.h"

int main(int argc, char **argv) {

  int opt;
  std::string filename;
  std::string detSpecFilename;
  std::string broker = "sakura";
  std::string instrumentName = "test";
  std::string compression = "";
  bool slow = false;
  bool quietMode = false;
  bool randomMode = false;
  bool singleRun = false;
  int maxEventsPerFramePart = 200;

  while ((opt = getopt(argc, argv, "f:d:b:i:c:m:squz")) != -1) {
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

    case 'm':
      maxEventsPerFramePart = std::stoi(optarg);
      break;

    case 's':
      slow = true;
      break;

    case 'q':
      quietMode = true;
      break;

    case 'u':
      randomMode = true;
      break;

    case 'z':
      singleRun = true;
      break;

    default:
      goto usage;
    }
  }

  if (filename.empty() || detSpecFilename.empty()) {
  usage:
    fprintf(stderr,
            "Usage:\n"
            "%s -f <filepath>    Full file path of nexus file to stream\n"
            "-d <det_spec_map_filepath>    Full file path of file defining the "
            "det-spec mapping\n"
            "[-b <host>]    Broker IP address or hostname, default is "
            "'sakura'\n"
            "[-i <instrument_name>]    Used as prefix for topic names\n"
            "[-m <max_events_per_message>]   Maximum number of events to send "
            "in a single message, default is 200\n"
            "[-s]    Slow mode, publishes data at approx realistic rate of 10 "
            "frames per second\n"
            "[-q]    Quiet mode, makes publisher less chatty on stdout\n"
            "[-u]    Random mode, serve messages within each frame in a random "
            "order\n"
            "[-z]    Produce only a single run (otherwise repeats until "
            "interrupted)"
            "\n",
            argv[0]);
    exit(1);
  }

  auto publisher = std::make_shared<KafkaEventPublisher>(compression);
  int runNumber = 1;
  NexusPublisher streamer(publisher, broker, instrumentName, filename,
                          detSpecFilename, quietMode, randomMode);

  // Publish the same data repeatedly, with incrementing run numbers
  if (singleRun) {
    streamer.streamData(maxEventsPerFramePart, runNumber, slow);
  } else {
    while (true) {
      streamer.streamData(maxEventsPerFramePart, runNumber, slow);
      runNumber++;
    }
  }

  return 0;
}
