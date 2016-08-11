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
  std::string broker = "sakura";
  std::string topic = "test_topic";
  std::string compression = "";
  bool slow = false;
  bool quietMode = false;
  int messagesPerFrame = 1;

  while ((opt = getopt(argc, argv, "f:b:t:c:m:sq")) != -1) {
    switch (opt) {

    case 'f':
      filename = optarg;
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

  if (filename.empty()) {
  usage:
    fprintf(stderr, "Usage: %s -f <filepath> "
                    "[-b <host:port>] "
                    "[-t <topic_name>] "
                    "[-m <messages_per_frame>] "
                    "[-s]"
                    "[-q] "
                    "\n",
            argv[0]);
    exit(1);
  }

  auto publisher = std::make_shared<KafkaEventPublisher>(compression);
  int runNumber = 1;
  NexusPublisher streamer(publisher, broker, topic, filename, quietMode);

  // Publish the same data repeatedly, with incrementing run numbers
  while (true) {
    streamer.streamData(messagesPerFrame, runNumber, slow);
    runNumber++;
  }

  return 0;
}
