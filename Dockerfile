FROM ubuntu:18.04

ARG local_conan_server

# Install build dependencies
ENV BUILD_PACKAGES "build-essential git python python-pip python-setuptools cmake"
RUN apt-get -y update && apt-get install $BUILD_PACKAGES -y --no-install-recommends

# Install other runtime dependencies
ENV RUN_PACKAGES "kafkacat google-perftools"
RUN apt-get -y update && apt-get install $RUN_PACKAGES -y --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

RUN pip install conan
# Force conan to create .conan directory and profile
RUN conan profile new default

RUN conan config install http://github.com/ess-dmsc/conan-configuration.git
ADD "https://raw.githubusercontent.com/ess-dmsc/docker-ubuntu18.04-build-node/master/files/default_profile" "/root/.conan/profiles/default"

RUN mkdir nexus_streamer
RUN mkdir nexus_streamer_src
COPY conan nexus_streamer_src/conan/
RUN cd nexus_streamer \
    && if [ ! -z "$local_conan_server" ]; then conan remote add --insert 0 local-conan-server "$local_conan_server"; fi \
    && conan install --build=outdated ../nexus_streamer_src/conan/conanfile.txt

# Build NeXus-Streamer
COPY cmake nexus_streamer_src/cmake/
COPY serialisation nexus_streamer_src/serialisation/
COPY nexus_file_reader nexus_streamer_src/nexus_file_reader/
COPY nexus_producer nexus_streamer_src/nexus_producer/
COPY core nexus_streamer_src/core/
COPY CMakeLists.txt nexus_streamer_src/CMakeLists.txt
RUN cd nexus_streamer && \
    cmake ../nexus_streamer_src -DCONAN=MANUAL -DCMAKE_BUILD_TYPE=Release && \
    make nexus-streamer -j8

# Clean up
RUN conan remove "*" -s -f
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES
RUN rm -rf /tmp/* /var/tmp/*

# Add directory to mount external data for docker image
RUN mkdir nexus_streamer/data

COPY docker/docker-start.sh nexus_streamer/docker-start.sh

# Add example data
COPY data/SANS_test.nxs nexus_streamer/SANS_test.nxs
COPY data/spectrum_gastubes_01.dat nexus_streamer/spectrum_gastubes_01.dat

WORKDIR nexus_streamer

CMD ["./docker-start.sh"]
