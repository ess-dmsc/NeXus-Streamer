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

void addNXentryToFile(hdf5::file::File &file, const std::string &entryName) {
  auto rootGroup = file.root();
  auto entryGroup = rootGroup.create_group(entryName);
  write_attribute<std::string>(entryGroup, "NX_class", "NXentry");
}

void addNXeventDataToFile(hdf5::file::File &file, const std::string &entryName,
                          const std::string &groupName) {
  hdf5::node::Group entryGroup = file.root()[entryName];
  auto eventGroup = entryGroup.create_group(groupName);
  write_attribute<std::string>(eventGroup, "NX_class", "NXevent_data");
}

void addNXeventDataDatasetsToFile(hdf5::file::File &file,
                                  const std::string &entryName) {
  addNXeventDataDatasetsToFile(file, {1}, {2}, {3}, {4}, entryName);
}

void addNXeventDataDatasetsToFile(hdf5::file::File &file,
                                  const std::vector<int64_t> &eventTimeZero,
                                  const std::vector<int32_t> &eventTimeOffset,
                                  const std::vector<uint64_t> &eventIndex,
                                  const std::vector<uint32_t> &eventId,
                                  const std::string &entryName,
                                  const std::string &groupName) {
  hdf5::node::Group eventGroup = file.root()[entryName + "/" + groupName];
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

void addVMSCompatGroupToFile(hdf5::file::File &file) {
  hdf5::node::Group entryGroup = file.root()["raw_data_1"];
  auto eventGroup = entryGroup.create_group("isis_vms_compat");
}

void addHistogramDataGroupToFile(hdf5::file::File &file,
                                 const std::string &entryName,
                                 const std::string &groupName,
                                 const std::vector<int32_t> &counts,
                                 const std::vector<int32_t> &detectorIDs,
                                 size_t periods, size_t tofBins,
                                 const std::vector<float> &tofBinEdges) {
  hdf5::node::Group entryGroup = file.root()[entryName];
  auto eventGroup = entryGroup.create_group(groupName);
  write_attribute<std::string>(eventGroup, "NX_class", "NXdata");

  auto spectrumIndexDataset = eventGroup.create_dataset(
      "spectrum_index", hdf5::datatype::create<int32_t>(),
      hdf5::dataspace::Simple({detectorIDs.size()}, {detectorIDs.size()}));
  spectrumIndexDataset.write(detectorIDs);

  auto countsDataset = eventGroup.create_dataset(
      "counts", hdf5::datatype::create<int32_t>(),
      hdf5::dataspace::Simple({periods, detectorIDs.size(), tofBins},
                              {periods, detectorIDs.size(), tofBins}));
  countsDataset.write(counts);
  countsDataset.attributes.create_from<std::string>("units", "counts");

  auto timeOfFlightDataset = eventGroup.create_dataset(
      "time_of_flight", hdf5::datatype::create<float>(),
      hdf5::dataspace::Simple({tofBinEdges.size()}, {tofBinEdges.size()}));
  timeOfFlightDataset.write(tofBinEdges);
  timeOfFlightDataset.attributes.create_from<std::string>("units",
                                                          "microseconds");
}

void addDurationDatasetToFile(hdf5::file::File &file,
                              const std::string &entryName, float duration,
                              const std::string &units) {
  hdf5::node::Group entryGroup = file.root()[entryName];
  auto durationDataset = entryGroup.create_dataset(
      "duration", hdf5::datatype::create<float>(), hdf5::dataspace::Scalar());
  durationDataset.write(duration);
  write_attribute(durationDataset, "units", units);
}
}
