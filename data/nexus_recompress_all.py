import h5py
import tables
import os
import time

"""
Read data from nexus file and write it to a new BLOSC compressed one.
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
    gzip_file_name = 'gzip_SANS_test.nxs'
    blosc_file_name = 'blosc_SANS_test.nxs'

    t0 = time.time()
    write_file(gzip_file_name, compress_type='gzip')
    t1 = time.time()
    write_file(blosc_file_name, compress_type=32001)  # 32001 is the Blosc filter
    t2 = time.time()

    # Time here includes reading the file
    print "Usual gzip compression took " + str(t1-t0) + " seconds and produced a file of " + str(os.stat(gzip_file_name).st_size) + " bytes"
    print "Blosc compression took " + str(t2-t1) + " seconds and produced a file of " + str(os.stat(blosc_file_name).st_size) + " bytes"
