#pragma once

#include <h5cpp/hdf5.hpp>

namespace HDF5FileTestHelpers {

/// Creates a file object in memory (does not get written to disk)
hdf5::file::File createInMemoryTestFile(const std::string &Filename);

/// Adds an NXentry group called "entry" to the root of the file
void addNXentryToFile(hdf5::file::File &file);

/// Adds an NXevent_data group called "detector_1_events" to the entry group
void addNXeventDataToFile(hdf5::file::File &file);

/// Adds good_frames dataset to file
void addGoodFramesToFile(hdf5::file::File &file);
}
