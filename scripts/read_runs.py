import datetime
import ISISDAE.RunInfo
from confluent_kafka import Consumer, KafkaError
"""
Print published run information from Kafka stream
"""


def parseMessage(buf):
    buf = bytearray(buf)
    runInfo = ISISDAE.RunInfo.RunInfo.GetRootAsRunInfo(buf, 0)
    start_time = datetime.datetime.fromtimestamp(runInfo.StartTime()).strftime('%Y-%m-%d %H:%M:%S')
    string_to_print = "Run number: " + str(runInfo.RunNumber()) + \
        ", Start time: " + start_time + \
        ", Instrument name: " + runInfo.InstName() + \
        ", Stream offset: " + str(runInfo.StreamOffset())
    print string_to_print


if __name__ == "__main__":
    c = Consumer({'bootstrap.servers': 'tenten', 'group.id': 'python-read-run-info',
                  'default.topic.config': {'auto.offset.reset': 'smallest'}, 'enable.auto.commit': False})
    c.subscribe(['test_run_topic'])
    running = True
    while running:
        msg = c.poll()
        if not msg.error():
            parseMessage(msg.value())
        elif msg.error().code() != KafkaError._PARTITION_EOF:
            print(msg.error())
            running = False
    c.close()
