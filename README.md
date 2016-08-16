[![Build Status](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid.svg?branch=master)](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid) [![Coverage Status](https://coveralls.io/repos/github/ScreamingUdder/isis_nexus_streamer_for_mantid/badge.svg?branch=master)](https://coveralls.io/github/ScreamingUdder/isis_nexus_streamer_for_mantid?branch=master) [![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ScreamingUdder/isis_nexus_streamer_for_mantid/blob/master/LICENSE)

# ISIS NeXus Streamer for Mantid
Stream event data from a NeXus file from RAL/ISIS using Apache Kafka for the purpose of development of live data streaming in Mantid.

Run metadata messages are sent to both the event stream and the run info stream. The offset recorded in the message is guaranteed to correspond to just before the RunInfo message in the event stream. Messages should be discarded until a RunInfo message with the same run number is encountered. In practice, at slow data rates and with a single producer the offset will match exactly.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers.

Usage:
```
main_nexusPublisher -f <filepath>    Full file path of nexus file to stream
-d <det_spec_map_filepath>    Full file path of file defining the det-spec mapping
[-b <host>]    Broker IP address or hostname, default is 'sakura'
[-t <event_topic_name>]    Name of event data topic to publish to, default is 'test_event_topic'
[-r <run_topic_name>]    Name of run data topic to publish to, default is 'test_run_topic'
[-a <det_spec_topic_name>]    Name of detector-spectra map topic to publish to, default is 'test_det_spec_topic'
[-m <messages_per_frame>]   Number of messages per frame to use, default is '1'
[-s]    Slow mode, publishes data at approx realistic rate of 10 frames per second
[-q]    Quiet mode, makes publisher less chatty on stdout
```

## Dependencies
Currently requires having `librdkafka` and the HDF5 C++ library installed. If `tcmalloc` is available then it will be used, but it is not a requirement.

`Google Test` and `Google Mock` are used for unit testing but are not required to be installed; CMake will download and build them at configure-time.

Due to large file sizes altering the test data requires `git lfs` to be installed:
https://git-lfs.github.com/

## Unit tests
The unit test executable `unitTests` needs to be passed the path of the test data directory as an argument.
Alternatively, run all units tests using ctest with
```
ctest -VV
```
from the build directory.
