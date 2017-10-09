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

# TODO launch publisher
