#!/bin/bash

# Wait for Kafka to be ready before starting the publisher
kafkacat -b kafka -L
OUT=$?
i="0"
while [ $OUT -ne 0 -a  $i -ne 5  ]; do
   echo "Waiting for Kafka to be ready"
   kafkacat -b localhost -L
   OUT=$?
   let i=$i+1
   echo $i
done
if [ $i -eq 5 ]
then
   echo "Kafka broker not accessible at producer launch"
   exit 1
fi

if [ -v NEXUS_FILE_NAME ]; then
   NEXUS_FILE_NAME="/nexus_publisher/data/$NEXUS_FILE_NAME"
else
   : ${NEXUS_FILE_NAME:=SANS_test.nxs}
fi

if [ -v DETSPECMAP_FILE_NAME ]; then
   DETSPECMAP_FILE_NAME="/nexus_publisher/data/$DETSPECMAP_FILE_NAME"
else
   : ${DETSPECMAP_FILE_NAME:=spectrum_gastubes_01.dat}
fi

: ${KAFKA_BROKER_NAME:=localhost}
: ${INSTRUMENT_NAME:=TEST}

ADDITIONAL_FLAGS=""
if [ ${SINGLE_RUN:="true"} == "true" ]; then
    ADDITIONAL_FLAGS="$ADDITIONAL_FLAGS -z"
fi
if [ ${SLOW_MODE:="false"} == "true" ]; then
    ADDITIONAL_FLAGS="$ADDITIONAL_FLAGS -s"
fi

nexus_producer/main_nexusPublisher -f ${NEXUS_FILE_NAME} -b ${KAFKA_BROKER_NAME} -i ${INSTRUMENT_NAME} -d ${DETSPECMAP_FILE_NAME} ${ADDITIONAL_FLAGS}
