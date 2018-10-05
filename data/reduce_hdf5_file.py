import h5py
import numpy as np

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
        time_dataset = '/raw_data_1/selog/{}/value_log/time'.format(dataset)
        value_dataset = '/raw_data_1/selog/{}/value_log/value'.format(dataset)
        data_time = f_read.get(time_dataset)
        data_value = f_read.get(value_dataset)

        with h5py.File(uncompressed_file, 'r+') as f_write:
            f_write[time_dataset] = data_time[...]
            f_write[value_dataset] = data_value[...]

    # Add fake int and long SE event data for using in unit tests
    with h5py.File(uncompressed_file, 'r+') as f_write:
        group_fake_int = f_write.get('/raw_data_1/selog').create_group('fake_int')
        group_fake_int_values = group_fake_int.create_group('value_log')
        group_fake_int_values.create_dataset('time', data=np.array([5.67, 23.5]).astype(np.float32))
        group_fake_int_values.create_dataset('value', data=np.array([1, 2]).astype(np.int32))
        group_fake_long = f_write.get('/raw_data_1/selog').create_group('fake_long')
        group_fake_long_values = group_fake_long.create_group('value_log')
        group_fake_long_values.create_dataset('time', data=np.array([5.67, 23.5]).astype(np.float32))
        group_fake_long_values.create_dataset('value', data=np.array([7, 42]).astype(np.int64))

    with h5py.File(uncompressed_file, 'r+') as f_write:
        f_write['/raw_data_1/good_frames'] = max_frame
        f_write['/raw_data_1/detector_1_events/total_counts'] = max_event
        f_write['/raw_data_1'].attrs['NX_class'] = "NXentry"
        f_write['/raw_data_1/detector_1_events'].attrs['NX_class'] = "NXevent_data"
        f_write['/raw_data_1/detector_1_events/event_time_zero'].attrs['offset'] = "2016-04-12T02:58:52"
