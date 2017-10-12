#!/bin/sh

# Wait for Kafka to be ready before starting the publisher
kafkacat -b kafka -L
OUT=$?
i="0"
while [ $OUT -ne 0 -a  $i -ne 10  ]; do
   echo "Waiting for Kafka to be ready"
   kafkacat -b kafka -L
   OUT=$?
   i=$[$i+1]
done

nexus_producer/main_nexusPublisher -f SANS_test.nxs -b kafka -i INSTR -d spectrum_gastubes_01.dat -z
