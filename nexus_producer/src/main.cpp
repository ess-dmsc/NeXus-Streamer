#ifdef _MSC_VER
#include "../win32/wingetopt.h"
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
  std::string topic = "test_event_topic";
  std::string runTopic = "test_run_topic";
  std::string detSpecTopic = "test_det_spec_topic";
  std::string compression = "";
  bool slow = false;
  bool quietMode = false;
  int messagesPerFrame = 1;

  while ((opt = getopt(argc, argv, "f:d:b:t:c:m:r:a:sq")) != -1) {
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

    case 't':
      topic = optarg;
      break;

    case 'c':
      compression = optarg;
      break;

    case 'm':
      messagesPerFrame = std::stoi(optarg);
      break;

    case 'r':
      runTopic = optarg;
      break;

    case 'a':
      detSpecTopic = optarg;
      break;

    case 's':
      slow = true;
      break;

    case 'q':
      quietMode = true;
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
            "[-t <event_topic_name>]    Name of event data topic to "
            "publish to, default is 'test_event_topic'\n"
            "[-r <run_topic_name>]    Name of run data topic to "
            "publish to, default is 'test_run_topic'\n"
            "[-a <det_spec_topic_name>]    Name of detector-spectra "
            "map topic to "
            "publish to, default is 'test_det_spec_topic'\n"
            "[-m <messages_per_frame>]   umber of messages per frame to use, "
            "default is '1'\n"
            "[-s]    Slow mode, publishes data at approx realistic rate of 10 "
            "frames per second\n"
            "[-q]    Quiet mode, makes publisher less chatty on stdout\n"
            "\n",
            argv[0]);
    exit(1);
  }

  auto publisher = std::make_shared<KafkaEventPublisher>(compression);
  int runNumber = 1;
  NexusPublisher streamer(publisher, broker, topic, runTopic, detSpecTopic,
                          filename, detSpecFilename, quietMode);

  // Publish the same data repeatedly, with incrementing run numbers
  while (true) {
    streamer.streamData(messagesPerFrame, runNumber, slow);
    runNumber++;
  }

  return 0;
}
