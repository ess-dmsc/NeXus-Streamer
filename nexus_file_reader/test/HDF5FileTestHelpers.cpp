#include "HDF5FileTestHelpers.h"

namespace HDF5FileTestHelpers {

hdf5::file::File createInMemoryTestFile(const std::string &filename) {
  hdf5::property::FileAccessList fapl;
  fapl.driver(hdf5::file::MemoryDriver());

  return hdf5::file::create(filename, hdf5::file::AccessFlags::TRUNCATE,
                            hdf5::property::FileCreationList(), fapl);
}

template <typename T>
static void write_attribute(hdf5::node::Node &node, const std::string &name,
                            T value) {
  hdf5::property::AttributeCreationList acpl;
  acpl.character_encoding(hdf5::datatype::CharacterEncoding::UTF8);
  node.attributes.create<T>(name, acpl).write(value);
}

void addNXentryToFile(hdf5::file::File &file) {
  auto rootGroup = file.root();
  auto entryGroup = rootGroup.create_group("entry");
  write_attribute<std::string>(entryGroup, "NX_class", "NXentry");
}

void addNXeventDataToFile(hdf5::file::File &file) {
  hdf5::node::Group entryGroup = file.root()["entry"];
  auto eventGroup = entryGroup.create_group("detector_1_events");
}

void addGoodFramesToFile(hdf5::file::File &file) {
  hdf5::node::Group entryGroup = file.root()["entry"];
  auto framesDataset = entryGroup.create_dataset(
      "good_frames", hdf5::datatype::create<int32_t>(),
      hdf5::dataspace::Scalar());
  framesDataset.write(10);
}
}
