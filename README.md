[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/NeXus-Streamer/blob/master/LICENSE) [![codecov](https://codecov.io/gh/ess-dmsc/NeXus-Streamer/branch/master/graph/badge.svg)](https://codecov.io/gh/ess-dmsc/NeXus-Streamer) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/)

# NeXus Streamer
Stream event data from a NeXus file to an Apache Kafka cluster. Each message sent over Kafka comprises the event data from a single neutron pulse. Using the `--slow` flag results in sending 10 messages per second, approximately a realistic rate. Sample environment data are also sent during streaming, resulting in a stream of real data which looks as if it were coming from a live instrument.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. However the `--single_run` flag can be used to produce only a single run.

Usage:
```
nexus-streamer <OPTIONS>

Options:
  -h,--help                   Print this help message and exit
  -f,--filename FILE REQUIRED Full path of the NeXus file
  -d,--det_spec_map FILE REQUIRED
                              Full path of the detector-spectrum map
  -b,--broker TEXT REQUIRED   Hostname or IP of Kafka broker
  -i,--instrument TEXT REQUIRED
                              Used as prefix for topic names
  -m,--compression TEXT       Compression option for Kafka messages
  -e,--fake_events_per_pulse INT
                              Generates this number of fake events per pulse instead of publishing real data from file
  -s,--slow                   Publish data at approx realistic rate (detected from file)
  -q,--quiet                  Less chatty on stdout
  -z,--single_run             Publish only a single run (otherwise repeats until interrupted)
  -c,--config_file TEXT       Read configuration from an ini file
```
Arguments not marked with `REQUIRED` are Optional. 

Usage example:
```
nexus-streamer --filename /path/to/NeXus-Streamer.git/data/SANS_test_uncompressed.hdf5 --det_spec_map /path/to/NeXus-Streamer.git/data/spectrum_gastubes_01.dat --broker localhost --instrument SANS2D --single_run
```

The NeXus Streamer can also be started using a configuration `ini` file with the `--config-file` argument, for example: 

```ini
filename=/path/to/nexus/file.nxs
det_spec_map=./paths/can/also/be/relative.dat
broker=localhost:9092
instrument=TEST
slow=true
```

Please note this requires the long argument, short arguments such as `-z` cannot be used in the `.ini` file.

## Minimum NeXus File Requirements
The minimum requirements of a NeXus file to be streamed are having an NXentry group (with any name) in the file root, containing a `name` dataset for the instrument name, and an NXevent_data group (with any name) containing `event_id`, `event_index`, `event_time_zero` and `event_time_offset` datasets. 
`/data/SANS2D_minimal.nxs` is an example of file meeting the minimum requirements.

## Broker Configuration
Timestamped "run" start and stop messages are produced. With these Mantid can join the stream at the start of a run and has various options for behaviour at run stop. This makes use of the offset by timestamp lookup feature and thus requires Kafka version >0.10.2.0 on the brokers.
It is also important to allow larger than the default message size by adding the following to the kafka configuration file (`server.properties`):
```
replica.fetch.max.bytes=10000000
message.max.bytes=10000000
```
## Instrument Name

The instrument name given using the `--instrument <INSTR>` command line option determines the names of the topics on which NeXus Streamer will publish data to these topics:
`INSTR_events` - neutron detection event data
`INSTR_sampleEnv` - sample environment log data
`INSTR_runInfo` - run start and stop messages
`INSTR_detSpecMap` - detector-spectrum map

Run start messages include an instrument name, this is used by Mantid to determine which Instrument Definition File (IDF) it should load. This instrument name is _not_ the one provided as a command line option, it is one found in the NeXus file in a string dataset at `/raw_data_1/name`. This should be the full, non-abbreviated name of the instrument, for example `SANS2D` or `LOKI`.

## Containers
The docker-compose script can be used to launch a single-broker Kafka cluster and the NeXus Streamer.
Run the following in the root directory of the repository to launch the containers.

```
docker-compose up
```
By default the streamer publishes some test data using the instrument name TEST. The Kafka broker is accessible at `localhost:9092`.

Pre-built containers are available at [Docker Hub](https://hub.docker.com/r/screamingudder/nexus-streamer/) tagged by the last commit on master at the time of building.  

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

If conan does not pick up your compiler settings, you can manually specify these by editing your conan profile.

for example to build with gcc 6.3 on Centos7 with c++11 support: 

```
[build_requires]
cmake_installer/3.10.0@conan/stable
[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=gcc
compiler.version=6.3
compiler.libcxx=libstdc++11
build_type=Release
[options]
[scopes]
[env]
```

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

## Profiling
CPU profiling can be performed by running the executable with `CPUPROFILE` environment variable set and specifying cmake parameter `PROFILE=true`.
For example:
```
CPUPROFILE=/tmp/prof.out <path/to/binary> [binary args]
```  
Use pprof to display the information. For example, to display as an SVG map in the browser:
```
google-pprof -web <path/to/binary> /tmp/prof.out
```
Note, this requires google perftools installed (tcmalloc and pprof). `gperftools` can be installed with Homebrew on OS X, or system repositories for most Linux distros.
