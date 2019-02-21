# Usage

Using the `--slow` flag results in results in data being published to Kafka at approximately a realistic rate, as if the instrument were running live. The pulse timestamps in the file are used to achieve this.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. However the `--single_run` flag can be used to produce only a single run.


Usage:
```
nexus-streamer <OPTIONS>

Options:
  -h,--help                   Print this help message and exit
  -f,--filename FILE REQUIRED Full path of the NeXus file
  -d,--det-spec-map FILE      Full path of the detector-spectrum map
  -b,--broker TEXT REQUIRED   Hostname or IP of Kafka broker
  -i,--instrument TEXT REQUIRED
                              Used as prefix for topic names
  -m,--compression TEXT       Compression option for Kafka messages
  -e,--fake-events-per-pulse INT
                              Generates this number of fake events per pulse per NXevent_data instead of publishing real data from file
  --histogram-update-period UINT
                              A histogram data message with this period (in integer milliseconds)
  -x,--disable-map INT INT    Use MIN and MAX detector numbers in inclusive range instead of using a det-spec map file
  -s,--slow                   Publish data at approx realistic rate (detected from file)
  -q,--quiet                  Less chatty on stdout
  -z,--single-run             Publish only a single run (otherwise repeats until interrupted)
  -c,--config-file TEXT       Read configuration from an ini file
```
Arguments not marked with `REQUIRED` are Optional.
A detector-spectrum map must be provided for use with Mantid.

Usage example:
```
nexus-streamer --filename /path/to/NeXus-Streamer.git/data/SANS_test_uncompressed.hdf5 --det-spec-map /path/to/NeXus-Streamer.git/data/spectrum_gastubes_01.dat --broker localhost --instrument SANS2D --single-run
```

The NeXus Streamer can also be started using a configuration `ini` file with the `--config-file` argument, for example:

```ini
filename=/path/to/nexus/file.nxs
det-spec-map=./paths/can/also/be/relative.dat
broker=localhost:9092
instrument=TEST
slow=true
```

There is an `ini` example file at `docker/example_config.ini`

Please note this requires the long argument, short arguments such as `-z` cannot be used in the `.ini` file.

## Minimum NeXus File Requirements
The minimum requirements of a NeXus file to be streamed are:
- `NXentry` group (with any name) in the file root
- A `name` dataset for the instrument name in the `NXentry` group

to publish event data:
- An `NXevent_data` group (with any name) in the `NXentry` containing `event_id`, `event_index`, `event_time_zero` and `event_time_offset` datasets. See `/data/SANS2D_minimal.nxs` as an example.

and/or to publish histogram data:
- A scalar float dataset named `duration` in the `NXentry` with a `units` attribute of "s", "seconds" or "second". This is the duration of the run and is used if slow mode is specified.
- An `NXdata` group in the `NXentry` containing a 

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
