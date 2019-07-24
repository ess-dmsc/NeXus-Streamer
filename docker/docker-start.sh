#!/usr/bin/env bash

# Wait for Kafka to be ready before starting the publisher
kafkacat -b ${KAFKA_BROKER:="localhost"} -L
OUT=$?
i="0"
while [[ ${OUT} -ne 0 -a  ${i} -ne 5  ]]; do
    echo "Waiting for Kafka to be ready"
    sleep 10
    kafkacat -b ${KAFKA_BROKER:="localhost"} -L
    OUT=$?
    let i=$i+1
    echo ${i}
done
if [[ ${i} -eq 5 ]]
then
    echo "Kafka broker not accessible at producer launch"
    exit 1
fi

# If we will publish geometry information then get NeXus filename from the config file
# and generate the JSON description using a python script
if [[ ${SEND_GEOMETRY} == 1 ]]; then
    while read -r -u3 line; do
        if [[ ${line} == *"filename"* ]]; then
          NEXUS_FILENAME=$(echo ${line} | awk -F = '{ print $2 }')
          echo "Generating JSON description of ${NEXUS_FILENAME}"
        fi
    done 3< "${CONFIG_FILE}"

    python3 generate_json_description.py -i ${NEXUS_FILENAME} -o nexus_structure_json.txt
fi

source /nexus_streamer/activate_run.sh
/nexus_streamer/bin/nexus-streamer -c ${CONFIG_FILE}
