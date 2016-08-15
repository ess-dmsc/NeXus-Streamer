import h5py

"""
Read data from compressed nexus file and write it to a new uncompressed one.
"""

compressed_file = 'SANS_test.nxs'
uncompressed_file = 'SANS_test_reduced.hdf5'

datasets_events = [
    '/raw_data_1/detector_1_events/event_id',
    '/raw_data_1/detector_1_events/event_time_offset'
]

datasets_frames = [
    '/raw_data_1/framelog/proton_charge/value',
    '/raw_data_1/detector_1_events/event_index',
    '/raw_data_1/detector_1_events/event_time_zero'
]

datasets_unchange = [
    '/raw_data_1/periods/number',
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

with h5py.File(uncompressed_file, 'w') as f_write:
    pass

max_event = 237078
max_frame = 300

with h5py.File(compressed_file, 'r') as f_read:
    for dataset in datasets_unchange:
        data_1 = f_read.get(dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            print(data_1[...])
            f_write[dataset] = data_1[...]

    for dataset in datasets_events:
        data_1 = f_read.get(dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            print(data_1[...])
            f_write[dataset] = data_1[0:max_event]

    for dataset in datasets_frames:
        data_1 = f_read.get(dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            print(data_1[...])
            f_write[dataset] = data_1[0:max_frame]

    for dataset in datasets_selogs:
        time_dataset = '/raw_data_1/selog/' + dataset + '/value_log/time'
        value_dataset = '/raw_data_1/selog/' + dataset + '/value_log/value'
        data_time = f_read.get(time_dataset)
        data_value = f_read.get(value_dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            f_write[time_dataset] = data_time[...]
            f_write[value_dataset] = data_value[...]

    with h5py.File(uncompressed_file, 'r+') as f_write:
        f_write['/raw_data_1/good_frames'] = max_frame
        f_write['/raw_data_1/detector_1_events/total_counts'] = max_event
