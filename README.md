[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/NeXus-Streamer/blob/master/LICENSE) [![codecov](https://codecov.io/gh/ess-dmsc/NeXus-Streamer/branch/master/graph/badge.svg)](https://codecov.io/gh/ess-dmsc/NeXus-Streamer) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/)

# NeXus Streamer
Stream event data from a NeXus file to an Apache Kafka cluster. Each message sent over Kafka comprises the event data from a single neutron pulse. Data in `NXlog`s, for example sample environment data, are also published.
Histogram data from NeXus files recorded at ISIS can also be streamed by setting `--histogram-update-period` to something higher than `0`.

Part of the ESS data streaming pipeline.

- [Further documentation](documentation/README.md)
  - [Usage](documentation/usage.md)
  - [Profiling](documentation/profiling.md)

### Geometry
A file can be provided with a json description of the NeXus file, using `--json-description`, this can include full geometry information about the instrument, which can be used by Mantid.
Further documentation and a utility for automatically generating the JSON description is included [here](generate_json/README.md).
A simple example NeXus file with geometry for a source, sample and detector is included at `data/SANS2D_minimal_with_geometry.nxs` 

## Getting Started

### Prerequisites
Dependencies are managed by [Conan](https://conan.io/). Conan can be installed using pip and CMake handles running Conan.
The following remote repositories are required to be configured:

- https://conan.bintray.com
- https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
- https://artifactoryconan.esss.dk/artifactory/api/conan/ecdc

You can add them by running
```
conan remote add <local-name> <remote-url>
```
where `<local-name>` must be substituted by a locally unique name. Configured
remotes can be listed with `conan remote list`.

If conan does not pick up your compiler settings, you can manually specify these by editing your conan profile.

for example to build with gcc 8.3 on Centos7:

```
[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=gcc
compiler.version=8.3
compiler.libcxx=libstdc++11
build_type=Release
[options]
[scopes]
[env]
```


### Building
As usual for a CMake project:
```
cmake <path-to-source>
make
```

There are some useful python scripts in the `data` directory for creating test data such as truncating large NeXus files or generating a detector-spectrum map file. 

## Running the tests
Build the CMake `UnitTests` target. Then use as follows:

```
UnitTests <OPTIONS>

Options:
  -h,--help                     Print this help message and exit
  -d,--data-path TEXT REQUIRED  Path to data directory
```

## Running via docker

The docker-compose script can be used to launch a single-broker Kafka cluster and the NeXus Streamer.
Run the following in the root directory of the repository to launch the containers.

```
docker-compose up
```
By default the streamer publishes some test data using the instrument name TEST. The Kafka broker is accessible at `localhost:9092`.
In [docker-compose.yml](docker-compose.yml) note the `SEND_GEOMETRY` option, set to 1 to automatically generate the JSON description of the NeXus file and include this in the run start message sent to Mantid. 

Pre-built containers are available at [Docker Hub](https://hub.docker.com/r/screamingudder/nexus-streamer/) tagged by the last commit on master at the time of building.  

## Built With
* [CMAKE](https://cmake.org/) - Cross platform makefile generation
* [Conan](https://conan.io/) - Package manager for C++
* [Docker](https://docker.com) - Container platform

## Authors

* **Matthew D Jones** - *Initial work* - [matthew-d-jones](https://github.com/matthew-d-jones)

See also the list of [contributors](https://github.com/ess-dmsc/NeXus-Streamer/graphs/contributors) who participated in this project.

## License

This project is licensed under the BSD-2 Clause License - see the [LICENSE.md](LICENSE.md) file for details
