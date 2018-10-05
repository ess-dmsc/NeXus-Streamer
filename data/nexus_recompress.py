import h5py
import tables
import os

"""
Read data from nexus file and write it to a new snappy compressed one.

NB, must build h5py from source after installing snappy for this to work
https://github.com/h5py/h5py

tables import is required to load blosc codec.
"""


def write_file(snappy_file, compress_type=32001, compress_opts=None):
    original_file = 'SANS_test.nxs'

    datasets = [
        '/raw_data_1/detector_1_events/event_id',
        '/raw_data_1/detector_1_events/total_counts',
        '/raw_data_1/detector_1_events/event_index',
        '/raw_data_1/good_frames',
        '/raw_data_1/detector_1_events/event_time_offset'
    ]

    with h5py.File(snappy_file, 'w') as f_write:
        pass

    with h5py.File(original_file, 'r') as f_read:
        for dataset in datasets:
            data_1 = f_read.get(dataset)

            with h5py.File(snappy_file, 'r+') as f_write:
                try:
                    d_set = f_write.create_dataset(dataset, data_1[...].shape, compression=compress_type,
                                                   compression_opts=compress_opts)
                    d_set[...] = data_1[...]
                except TypeError:
                    # probably this is a scalar dataset so just write it without compression
                    f_write[dataset] = data_1[...]


if __name__ == '__main__':
    import timeit

    print("timing with blosc:\n ")
    print("{}s for 10 repetitions".format(timeit.timeit("write_file('SANS_test_blosc.hdf5', compress_type=32001)", number=10,                                                           setup="from __main__ import write_file")))
    print("{}bytes".format(os.path.getsize('SANS_test_blosc.hdf5')))
    print("timing with gzip level 1:")
    print("{}s for 10 repetitions".format(timeit.timeit("write_file('SANS_test_gzip.hdf5', compress_type='gzip', compress_opts=1)", number=10,
                                                         setup="from __main__ import write_file")))
    print("{}bytes".format(os.path.getsize('SANS_test_gzip.hdf5')))
    print("timing with no compression:")
    print("{}s for 10 repetitions".format(timeit.timeit("write_file('SANS_test_noComp.hdf5', compress_type=None)", number=10,
                                                           setup="from __main__ import write_file")))
    print("{}bytes".format(os.path.getsize('SANS_test_noComp.hdf5')))
