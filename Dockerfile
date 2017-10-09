FROM ubuntu:17.04

ARG LIBRDKAFKA_VER="0.11.0"

RUN apt-get -y update

# Build librdkafka
ENV BUILD_PACKAGES "build-essential zlib1g-dev unzip python"
RUN apt-get install $BUILD_PACKAGES -y
ADD https://github.com/edenhill/librdkafka/archive/v$LIBRDKAFKA_VER.zip /tmp/source.zip
RUN cd /tmp && \
    unzip source.zip && mv librdkafka-* librdkafka && \
    cd /tmp/librdkafka && \
    ./configure && \
    make all && make install && \
    make clean && ./configure --clean
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES

WORKDIR /build

# Build kafkacat (to be used to check Kafka broker is running before launching clients)
ENV BUILD_PACKAGES "build-essential git curl zlib1g-dev python"
RUN apt-get install $BUILD_PACKAGES -y
RUN git clone https://github.com/edenhill/kafkacat.git
RUN cd kafkacat && \
    ./bootstrap.sh && \
    make install && \
    cd .. && rm -rf kafkacat
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES

# Build NeXus file streamer
# dependencies - libhdf5, build-essential, git
ENV BUILD_PACKAGES "build-essential git libhdf5-dev cmake"
RUN apt-get install $BUILD_PACKAGES -y
RUN git clone https://github.com/ScreamingUdder/isis_nexus_streamer_for_mantid.git
RUN mkdir nexus_publisher && \
    cd nexus_publisher && \
    cmake ../isis_nexus_streamer_for_mantid && \
    make
RUN AUTO_ADDED_PACKAGES=`apt-mark showauto`
RUN apt-get remove --purge -y $BUILD_PACKAGES $AUTO_ADDED_PACKAGES

# Clean up
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ADD docker-start.sh /nexus_publisher
ADD data/SANS_test.nxs /nexus_publisher
ADD data/spectrum_gastubes_01.dat /nexus_publisher
WORKDIR /nexus_publisher

CMD ["./docker-start.sh"]
