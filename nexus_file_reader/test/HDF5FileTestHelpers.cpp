#include "HDF5FileTestHelpers.h"

namespace HDF5FileTestHelpers {

hdf5::file::File createInMemoryTestFile(const std::string &Filename) {
  hdf5::property::FileAccessList fapl;
  fapl.driver(hdf5::file::MemoryDriver());

  return hdf5::file::create(Filename, hdf5::file::AccessFlags::TRUNCATE,
                            hdf5::property::FileCreationList(), fapl);
}
}
