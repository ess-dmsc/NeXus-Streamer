[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/NeXus-Streamer/blob/master/LICENSE) [![codecov](https://codecov.io/gh/ess-dmsc/NeXus-Streamer/branch/master/graph/badge.svg)](https://codecov.io/gh/ess-dmsc/NeXus-Streamer) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/)

# NeXus Streamer
Stream event data from a NeXus file to an Apache Kafka cluster. Each message sent over Kafka comprises the event data from a single neutron pulse. Using the `--slow` flag results in sending 10 messages per second, approximatelya realistic rate. Sample environment data are also sent during streaming, resulting in a stream of real data which looks as if it were coming from a live instrument.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. However the `--singlerun` flag can be used to produce only a single run.

Usage:
```
main_nexusPublisher <OPTIONS>

Options:
  -h,--help                   Print this help message and exit
  --filename TEXT             Full path of the NeXus file
  --detspecmap TEXT           Full path of the detector-spectrum map
  --broker TEXT               Hostname or IP of Kafka broker
  --instrument TEXT           Used as prefix for topic names
  --compression TEXT          Compression option for Kafka messages
  --slow                      Publish data at approx realistic rate (10 pulses per second)
  --quiet                     Less chatty on stdout
  --singlerun                 Publish only a single run (otherwise repeats until interrupted)
```

Usage example:
```
main_nexusPublisher --filename /path/to/NeXus-Streamer.git/data/SANS_test_uncompressed.hdf5 --detspecmap /path/to/NeXus-Streamer.git/data/spectrum_gastubes_01.dat --broker localhost --instrument SANS2D --singlerun
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
By default the streamer publishes some test data using the instrument name TEST. The Kafka broker is accessible at `localhost:9092`.

## Dependencies

Dependencies are managed by [Conan](https://conan.io/). Conan can be installed using pip and CMake handles running Conan.
The following remote repositories are required to be configured:

- https://api.bintray.com/conan/ess-dmsc/conan
- https://api.bintray.com/conan/conan-community/conan
- https://api.bintray.com/conan/bincrafters/public-conan

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
The unit test executable `UnitTests` needs to be passed the path of the test data directory as an argument.
Alternatively, run all units tests using ctest with
```
ctest -VV
```
from the build directory.

## Schema
The message schema files are located in https://github.com/ess-dmsc/streaming-data-types
