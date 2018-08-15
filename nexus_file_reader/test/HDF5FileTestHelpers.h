#pragma once

#include <h5cpp/hdf5.hpp>

namespace HDF5FileTestHelpers {

hdf5::file::File createInMemoryTestFile(const std::string &Filename);
}
