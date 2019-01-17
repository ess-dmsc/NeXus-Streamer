# Usage

Using the `--slow` flag results in results in data being published to Kafka at approximately a realistic rate, as if the instrument were running live. The pulse timestamps in the file are used to achieve this.

The client runs until the user terminates it, repeatedly sending data from the same file but with incrementing run numbers. However the `--single_run` flag can be used to produce only a single run.


Usage:
```
nexus-streamer <OPTIONS>

Options:
  -h,--help                   Print this help message and exit
  -f,--filename FILE REQUIRED Full path of the NeXus file
  -d,--det-spec-map FILE
                              Full path of the detector-spectrum map
  -b,--broker TEXT REQUIRED   Hostname or IP of Kafka broker
  -i,--instrument TEXT REQUIRED
                              Used as prefix for topic names
  -m,--compression TEXT       Compression option for Kafka messages
  -e,--fake-events-per-pulse INT
                              Generates this number of fake events per pulse instead of publishing real data from file
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
