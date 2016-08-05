import h5py
import tables
import os

"""
Read data from nexus file and write it to a new snappy compressed one.

NB, must build h5py from source after installing snappy for this to work
https://github.com/h5py/h5py
"""


def write_file(output_file, compress_type=32001, compress_opts=None):
    original_file = 'SANS_test.nxs'

    with h5py.File(output_file, 'w') as f_write:
        with h5py.File(original_file, 'r') as f_read:

            def write_in_new_file(name):
                dset = f_read[name]
                if isinstance(dset, h5py.Dataset):
                    try:
                        print name
                        output_dataset = f_write.create_dataset(name, dset[...].shape, compression=compress_type,
                                                                compression_opts=compress_opts)
                        output_dataset[...] = dset[...]
                    except TypeError:
                        # probably this is a scalar dataset so just write it without compression
                        f_write[name] = dset[...]
                    except IOError:
                        pass
                else:
                    print name
                    f_write.create_group(name)

            f_read.visit(write_in_new_file)


if __name__ == '__main__':
    write_file("very_temp.nxs")
