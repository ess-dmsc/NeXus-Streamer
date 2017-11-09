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

nexus_producer/main_nexusPublisher -f SANS_test.nxs -b localhost -i TEST -d spectrum_gastubes_01.dat -z
