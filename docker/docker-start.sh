#!/usr/bin/env bash

# Wait for Kafka to be ready before starting the publisher
kafkacat -b ${KAFKA_BROKER:="localhost"} -L
OUT=$?
i="0"
while [ $OUT -ne 0 -a  $i -ne 5  ]; do
   echo "Waiting for Kafka to be ready"
   sleep 10
   kafkacat -b ${KAFKA_BROKER:="localhost"} -L
   OUT=$?
   let i=$i+1
   echo $i
done
if [ $i -eq 5 ]
then
   echo "Kafka broker not accessible at producer launch"
   exit 1
fi

source /nexus_streamer/activate_run.sh
/nexus_streamer/bin/nexus-streamer -c ${CONFIG_FILE}
