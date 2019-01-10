[![License (2-Clause BSD)](https://img.shields.io/badge/license-BSD%202--Clause-blue.svg)](https://github.com/ess-dmsc/NeXus-Streamer/blob/master/LICENSE) [![codecov](https://codecov.io/gh/ess-dmsc/NeXus-Streamer/branch/master/graph/badge.svg)](https://codecov.io/gh/ess-dmsc/NeXus-Streamer) [![Build Status](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/badge/icon)](https://jenkins.esss.dk/dm/job/ess-dmsc/job/NeXus-Streamer/job/master/)

# NeXus Streamer
Stream event data from a NeXus file to an Apache Kafka cluster. Each message sent over Kafka comprises the event data from a single neutron pulse. Sample environment data are also published.

Part of the ESS data streaming pipeline.

- [Further documentation](documentation/README.md)

## Getting Started

### Prerequisites
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


### Installing
As usual for a CMake project:
```
cmake <path-to-source>
make
```

## Running the tests
The unit test executable `UnitTests` needs to be passed the path of the test data directory as an argument.
Alternatively, run all units tests using ctest with
```
ctest -VV
```
from the build directory.


## Deployment

The docker-compose script can be used to launch a single-broker Kafka cluster and the NeXus Streamer.
Run the following in the root directory of the repository to launch the containers.

```
docker-compose up
```
By default the streamer publishes some test data using the instrument name TEST. The Kafka broker is accessible at `localhost:9092`.

Pre-built containers are available at [Docker Hub](https://hub.docker.com/r/screamingudder/nexus-streamer/) tagged by the last commit on master at the time of building.  

## Built With
* [CMAKE](https://cmake.org/) - Cross platform makefile generation
* [Conan](https://conan.io/) - Package manager for C++
* [Docker](https://docker.com) - Container platform

## Contributing
TBD

## Versioning
TBD

## Authors

* **Matthew D Jonesr** - *Initial work* - [matthew-d-jones](https://github.com/matthew-d-jones)

See also the list of [contributors](https://github.com/ess-dmsc/NeXus-Streamer/graphs/contributors) who participated in this project.

## License

This project is licensed under the BSD-2 Clause License - see the [LICENSE.md](LICENSE.md) file for details
