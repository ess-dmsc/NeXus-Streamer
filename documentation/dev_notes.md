

## Minimum NeXus File Requirements
The minimum requirements of a NeXus file to be streamed are having an NXentry group (with any name) in the file root, containing a `name` dataset for the instrument name, and an NXevent_data group (with any name) containing `event_id`, `event_index`, `event_time_zero` and `event_time_offset` datasets.
`/data/SANS2D_minimal.nxs` is an example of file meeting the minimum requirements.

## Broker Configuration
Timestamped "run" start and stop messages are produced. With these Mantid can join the stream at the start of a run and has various options for behaviour at run stop. This makes use of the offset by timestamp lookup feature and thus requires Kafka version >0.10.2.0 on the brokers.
It is also important to allow larger than the default message size by adding the following to the kafka configuration file (`server.properties`):
```
replica.fetch.max.bytes=10000000
message.max.bytes=10000000
```
## Instrument Name

The instrument name given using the `--instrument <INSTR>` command line option determines the names of the topics on which NeXus Streamer will publish data to these topics:
`INSTR_events` - neutron detection event data
`INSTR_sampleEnv` - sample environment log data
`INSTR_runInfo` - run start and stop messages
`INSTR_detSpecMap` - detector-spectrum map

Run start messages include an instrument name, this is used by Mantid to determine which Instrument Definition File (IDF) it should load. This instrument name is _not_ the one provided as a command line option, it is one found in the NeXus file in a string dataset at `/raw_data_1/name`. This should be the full, non-abbreviated name of the instrument, for example `SANS2D` or `LOKI`.


## Schema
The message schema files are located in https://github.com/ess-dmsc/streaming-data-types
