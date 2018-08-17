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
  write_attribute<std::string>(eventGroup, "NX_class", "NXevent_data");
}

void addNXeventDataDatasetsToFile(hdf5::file::File &file) {
  hdf5::node::Group eventGroup = file.root()["entry/detector_1_events"];
  auto eventTimeZero = eventGroup.create_dataset(
      "event_time_zero", hdf5::datatype::create<int64_t>(),
      hdf5::dataspace::Scalar());
  eventTimeZero.write(1);

  auto eventTimeOffset = eventGroup.create_dataset(
      "event_time_offset", hdf5::datatype::create<int32_t>(),
      hdf5::dataspace::Scalar());
  eventTimeOffset.write(2);

  auto eventIndex = eventGroup.create_dataset(
      "event_index", hdf5::datatype::create<uint64_t>(),
      hdf5::dataspace::Scalar());
  eventIndex.write(3);

  auto eventId =
      eventGroup.create_dataset("event_id", hdf5::datatype::create<uint32_t>(),
                                hdf5::dataspace::Scalar());
  eventId.write(4);
}
}
