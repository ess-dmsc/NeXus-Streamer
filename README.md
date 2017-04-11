[![Build Status](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid.svg?branch=master)](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid) [![Build Status](https://ci.appveyor.com/api/projects/status/1oqx295j13frpj9w?svg=true)](https://ci.appveyor.com/project/matthew-d-jones/isis-nexus-streamer-for-mantid) [![Coverage Status](https://coveralls.io/repos/github/ScreamingUdder/isis_nexus_streamer_for_mantid/badge.svg?branch=master)](https://coveralls.io/github/ScreamingUdder/isis_nexus_streamer_for_mantid?branch=master) [![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ScreamingUdder/isis_nexus_streamer_for_mantid/blob/master/LICENSE)

# ISIS NeXus Streamer for Mantid
Stream event data from a NeXus file from RAL/ISIS using Apache Kafka for the purpose of development of live data streaming in Mantid.

Run start and stop messages are produced, to make use of these and consume from a specified timestamp Kafka version >0.10.2.0 is required.

A variable number of messages are sent per frame, such that the number of events in a message does not exceed a limit. The limit is 200 by default and can be specified using the optional argument `-m <max_events_per_message>`.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. Unless the `-z` flag is used to produce only a single run.

Usage:
```
main_nexusPublisher -f <filepath>    Full file path of nexus file to stream
-d <det_spec_map_filepath>    Full file path of file defining the det-spec mapping
[-b <host>]    Broker IP address or hostname, default is 'sakura'
[-i <instrument_name>]    Used as prefix for topic names
[-m <max_events_per_message>]   Maximum number of events to send in a single message, default is '200'
[-s]    Slow mode, publishes data at approx realistic rate of 10 frames per second
[-q]    Quiet mode, makes publisher less chatty on stdout
[-u]    Random mode, serve messages within each frame in a random order, for testing purposes
[-z]    Produce only a single run (otherwise repeats until interrupted)
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

## Schema
The schema files are located in https://github.com/ess-dmsc/streaming-data-types
