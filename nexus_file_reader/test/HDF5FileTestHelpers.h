#pragma once

#include <h5cpp/hdf5.hpp>

namespace HDF5FileTestHelpers {

/// Creates a file object in memory (does not get written to disk)
hdf5::file::File createInMemoryTestFile(const std::string &Filename);

/// Creates a file object in memory, containing event data
hdf5::file::File
createInMemoryTestFileWithEventData(const std::string &filename);

/// Adds an NXentry group called "entry" to the root of the file
void addNXentryToFile(hdf5::file::File &file);

/// Adds an NXevent_data group called "detector_1_events" to the entry group
void addNXeventDataToFile(hdf5::file::File &file);

/// Adds datasets to event data group in file
void addNXeventDataDatasetsToFile(hdf5::file::File &file);

void addNXeventDataDatasetsToFile(hdf5::file::File &file,
                                  const std::vector<int64_t> &eventTimeZero,
                                  const std::vector<int32_t> &eventTimeOffset,
                                  const std::vector<uint64_t> &eventIndex,
                                  const std::vector<uint32_t> &eventId);
}
