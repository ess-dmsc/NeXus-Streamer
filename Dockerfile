FROM ubuntu:18.04

# Install build dependencies
ENV BUILD_PACKAGES "build-essential git python python-dev cmake"
RUN apt-get -y update && apt-get install $BUILD_PACKAGES -y --no-install-recommends

# Install other runtime dependencies
ENV RUN_PACKAGES "kafkacat google-perftools"
RUN apt-get -y update && apt-get install $RUN_PACKAGES -y --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

# Build NeXus-Streamer
RUN mkdir nexus_streamer_src
ADD . nexus_streamer_src/
RUN mkdir nexus_streamer && \
    cd nexus_streamer && \
    cmake ../nexus_streamer_src && \
    make -j8

# Clean up
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES
RUN rm -rf /tmp/* /var/tmp/*

# Add directory to mount external data for docker image
RUN mkdir nexus_streamer/data

ADD docker-start.sh nexus_streamer/docker-start.sh
ADD data/SANS_test.nxs nexus_streamer/SANS_test.nxs
ADD data/spectrum_gastubes_01.dat nexus_streamer/spectrum_gastubes_01.dat
WORKDIR nexus_streamer

CMD ["./docker-start.sh"]
