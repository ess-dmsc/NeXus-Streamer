[![Build Status](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid.svg?branch=master)](https://travis-ci.org/ScreamingUdder/isis_nexus_streamer_for_mantid) [![Coverage Status](https://coveralls.io/repos/github/ScreamingUdder/isis_nexus_streamer_for_mantid/badge.svg?branch=master)](https://coveralls.io/github/ScreamingUdder/isis_nexus_streamer_for_mantid?branch=master) [![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ScreamingUdder/isis_nexus_streamer_for_mantid/blob/master/LICENSE)

# ISIS NeXus Streamer for Mantid
Stream event data from a NeXus file from RAL/ISIS using Apache Kafka for the purposes of development of live data streaming in Mantid.

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
