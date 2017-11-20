FROM ubuntu:17.04

ARG LIBRDKAFKA_VER="0.11.1"
ARG KAFKACAT_VER="1.3.1"

# Install dependencies
ENV BUILD_PACKAGES "build-essential git curl zlib1g-dev python libhdf5-dev cmake"
RUN apt-get -y update && apt-get install $BUILD_PACKAGES -y --no-install-recommends && \
    rm -rf /var/lib/apt/lists/*

# Build librdkafka
ADD https://github.com/edenhill/librdkafka/archive/v$LIBRDKAFKA_VER.tar.gz /tmp/source.tar.gz
RUN cd /tmp && \
    tar -xzf source.tar.gz && mv librdkafka-* librdkafka && \
    cd /tmp/librdkafka && \
    ./configure && \
    make all && make install && \
    make clean && ./configure --clean && \
    ldconfig

# Build kafkacat (to be used to check Kafka broker is running before launching clients)
ADD https://github.com/edenhill/kafkacat/archive/$KAFKACAT_VER.tar.gz /tmp/kafkacat_source.tar.gz
RUN cd /tmp && \
    tar -xzf kafkacat_source.tar.gz && \
    mv kafkacat-* kafkacat && \
    cd /tmp/kafkacat && sync && \
    ./configure && make && \
    make install && \
    cd .. && rm -rf kafkacat

# Build NeXus file streamer
# dependencies - libhdf5, build-essential
RUN mkdir isis_nexus_streamer_for_mantid
ADD . isis_nexus_streamer_for_mantid/
RUN mkdir nexus_publisher && \
    cd nexus_publisher && \
    git config --global http.sslVerify false && \
    cmake ../isis_nexus_streamer_for_mantid && \
    make

# Clean up
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES
RUN rm -rf /tmp/* /var/tmp/*

# Add directory to mount external data for docker image
RUN mkdir nexus_publisher/data

ADD docker-start.sh nexus_publisher/docker-start.sh
ADD data/SANS_test.nxs nexus_publisher/SANS_test.nxs
ADD data/spectrum_gastubes_01.dat nexus_publisher/spectrum_gastubes_01.dat
WORKDIR nexus_publisher

CMD ["./docker-start.sh"]
