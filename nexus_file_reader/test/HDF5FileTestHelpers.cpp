#include "HDF5FileTestHelpers.h"

namespace HDF5FileTestHelpers {

hdf5::file::File createInMemoryTestFile(const std::string &filename) {
  hdf5::property::FileAccessList fapl;
  fapl.driver(hdf5::file::MemoryDriver());

  return hdf5::file::create(filename, hdf5::file::AccessFlags::TRUNCATE,
                            hdf5::property::FileCreationList(), fapl);
}

hdf5::file::File
createInMemoryTestFileWithEventData(const std::string &filename) {
  auto file = createInMemoryTestFile(filename);
  HDF5FileTestHelpers::addNXentryToFile(file);
  HDF5FileTestHelpers::addNXeventDataToFile(file);
  HDF5FileTestHelpers::addNXeventDataDatasetsToFile(file);
  return file;
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
  addNXeventDataDatasetsToFile(file, {1}, {2}, {3}, {4});
}

void addNXeventDataDatasetsToFile(hdf5::file::File &file,
                                  const std::vector<int64_t> &eventTimeZero,
                                  const std::vector<int32_t> &eventTimeOffset,
                                  const std::vector<uint64_t> &eventIndex,
                                  const std::vector<uint32_t> &eventId) {
  hdf5::node::Group eventGroup = file.root()["entry/detector_1_events"];
  auto eventTimeZeroDataset = eventGroup.create_dataset(
      "event_time_zero", hdf5::datatype::create<int64_t>(),
      hdf5::dataspace::Simple({eventTimeZero.size()}, {eventTimeZero.size()}));
  eventTimeZeroDataset.write(eventTimeZero);

  auto eventTimeOffsetDataset = eventGroup.create_dataset(
      "event_time_offset", hdf5::datatype::create<int32_t>(),
      hdf5::dataspace::Simple({eventTimeOffset.size()},
                              {eventTimeOffset.size()}));
  eventTimeOffsetDataset.write(eventTimeOffset);

  auto eventIndexDataset = eventGroup.create_dataset(
      "event_index", hdf5::datatype::create<uint64_t>(),
      hdf5::dataspace::Simple({eventIndex.size()}, {eventIndex.size()}));
  eventIndexDataset.write(eventIndex);

  auto eventIdDataset = eventGroup.create_dataset(
      "event_id", hdf5::datatype::create<uint32_t>(),
      hdf5::dataspace::Simple({eventId.size()}, {eventId.size()}));
  eventIdDataset.write(eventId);
}
}
