import h5py

"""
Read data from compressed nexus file and write it to a new uncompressed one.
"""

compressed_file = 'SANS_test.nxs'
uncompressed_file = 'SANS_test_uncompressed.hdf5'

datasets = [
    '/raw_data_1/detector_1_events/event_id',
    '/raw_data_1/detector_1_events/total_counts',
    '/raw_data_1/detector_1_events/event_index',
    '/raw_data_1/good_frames',
    '/raw_data_1/detector_1_events/event_time_offset',
    'raw_data_1/framelog/proton_charge/value',
    '/raw_data_1/periods/number',
    '/raw_data_1/detector_1_events/event_time_zero',
    '/raw_data_1/name',
    '/raw_data_1/start_time'
]

datasets_selogs = [
    'Guide_Pressure',
    'Rear_Det_X',
    'SECI_OUT_OF_RANGE_BLOCK',
    'Sample',
    'TEMP1'
]

for dataset in datasets_selogs:
    datasets.append('/raw_data_1/selog/{}/value_log/time'.format(dataset))
    datasets.append('/raw_data_1/selog/{}/value_log/value'.format(dataset))

with h5py.File(uncompressed_file, 'w') as f_write:
    pass

with h5py.File(compressed_file, 'r') as f_read:
    for dataset in datasets:
        data_1 = f_read.get(dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            print(data_1[...])
            f_write[dataset] = data_1[...]
