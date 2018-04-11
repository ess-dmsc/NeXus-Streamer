[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/NeXus-Streamer/blob/master/LICENSE)

# NeXus Streamer
Stream event data from a NeXus file to an Apache Kafka cluster. Each message sent over Kafka comprises the event data from a single neutron pulse.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. However the `-z` flag can be used to produce only a single run.

Usage:
```
main_nexusPublisher -f <filepath>    Full file path of nexus file to stream
-d <det_spec_map_filepath>    Full file path of file defining the det-spec mapping
-b <host>    Broker IP address or hostname
[-i <instrument_name>]    Used as prefix for topic names
[-s]    Slow mode, publishes data at approx realistic rate of 10 frames per second
[-q]    Quiet mode, makes publisher less chatty on stdout
[-z]    Produce only a single run (otherwise repeats until interrupted)
```

Usage example:
```
main_nexusPublisher -f /path/to/isis_nexus_streamer_for_mantid.git/data/SANS_test_uncompressed.hdf5 -d /path/to/nexus_streamer.git/data/spectrum_gastubes_01.dat -b localhost -i SANS2D -z
```

## Broker Configuration
Timestamped "run" start and stop messages are produced. With these Mantid can join the stream at the start of a run and has various options for behaviour at run stop. This makes use of the offset by timestamp lookup feature and thus requires Kafka version >0.10.2.0 on the brokers.
It is also important to allow larger than the default message size by adding the following to the kafka configuration file (`server.properties`):
```
replica.fetch.max.bytes=10000000
message.max.bytes=10000000
```

## Containers
The docker-compose script can be used to launch a single-broker Kafka cluster and the NeXus Streamer.
Run the following in the root directory of the repository to launch the containers.

```
docker-compose up
```
The streamer publishes some test data using the instrument name TEST. The Kafka broker is accessible at `localhost:9092`.

## Dependencies

Dependencies can be installed using Conan. Conan can be installed using pip and CMake handles running Conan.
The following remote repositories are required to be configured:

- https://api.bintray.com/conan/ess-dmsc/conan
- https://api.bintray.com/conan/conan-community/conan

You can add them by running
```
conan remote add <local-name> <remote-url>
```
where `<local-name>` must be substituted by a locally unique name. Configured
remotes can be listed with `conan remote list`.

## Build

As usual for a CMake project:
```
cmake <path-to-source>
make
```

## Unit tests
The unit test executable `unitTests` needs to be passed the path of the test data directory as an argument.
Alternatively, run all units tests using ctest with
```
ctest -VV
```
from the build directory.

## Schema
The message schema files are located in https://github.com/ess-dmsc/streaming-data-types
