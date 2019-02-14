#include "HistogramData.h"
#include "../../core/include/Message.h"
#include "hs00_event_histogram_generated.h"

Streamer::Message createHistogramMessage(const std::vector<int32_t> &counts,
                                         const std::vector<size_t> &countsShape,
                                         const std::vector<float> &timeOfFlight,
                                         uint64_t timestampUnix) {
  flatbuffers::FlatBufferBuilder builder;

  auto periodsDimension =
      CreateDimensionMetaData(builder, static_cast<uint32_t>(countsShape[0]), 0,
                              builder.CreateString("Period"));

  auto timeOfFlightDimension = CreateDimensionMetaData(
      builder, static_cast<uint32_t>(countsShape[1]),
      builder.CreateString("us"), builder.CreateString("Time Of Flight"),
      Array::ArrayFloat,
      CreateArrayFloat(builder, builder.CreateVector(timeOfFlight)).Union());

  auto detectorsDimension =
      CreateDimensionMetaData(builder, static_cast<uint32_t>(countsShape[2]), 0,
                              builder.CreateString("Detector Number"));

  std::vector<flatbuffers::Offset<DimensionMetaData>> dimensionsArray{
      periodsDimension, timeOfFlightDimension, detectorsDimension};

  const std::vector<uint32_t> countsShapeUInt{
      static_cast<uint32_t>(countsShape[0]),
      static_cast<uint32_t>(countsShape[1]),
      static_cast<uint32_t>(countsShape[2])};

  const std::vector<uint32_t> offsets{0, 0, 0};

  std::vector<uint32_t> countsUInt(counts.cbegin(), counts.cend());

  auto histogramDataOffset = CreateEventHistogram(
      builder, builder.CreateString("NeXus-Streamer"), timestampUnix,
      builder.CreateVector(dimensionsArray), timestampUnix,
      builder.CreateVector(countsShapeUInt), builder.CreateVector(offsets),
      Array::ArrayUInt,
      CreateArrayUInt(builder, builder.CreateVector(countsUInt)).Union());

  FinishEventHistogramBuffer(builder, histogramDataOffset);

  return Streamer::Message(builder.Release());
}

/**
 * @param message : input message to be deserialised
 * @param counts : output by reference
 * @param countsShape : output by reference
 * @param timeOfFlight : output by reference
 * @param timestampUnix : output by reference
 */
void deserialiseHistogramMessage(Streamer::Message &message,
                                 std::vector<int32_t> &counts,
                                 std::vector<size_t> &countsShape,
                                 std::vector<float> &timeOfFlight,
                                 uint64_t &timestampUnix) {
  auto messageData = GetEventHistogram(message.data());
  timestampUnix = messageData->timestamp();

  countsShape.resize(3);

  auto dimMetadataArray = messageData->dim_metadata();
  for (auto dimMetadata : *dimMetadataArray) {
    if (dimMetadata->label()->str() == "Period") {
      countsShape[0] = static_cast<size_t>(dimMetadata->length());
    } else if (dimMetadata->label()->str() == "Time Of Flight") {
      countsShape[1] = static_cast<size_t>(dimMetadata->length());
      auto timeOfFlightArray = dimMetadata->bin_boundaries_as_ArrayFloat();
      timeOfFlight.resize(timeOfFlightArray->value()->size());
      std::copy(timeOfFlightArray->value()->begin(),
                timeOfFlightArray->value()->end(), timeOfFlight.begin());
    } else if (dimMetadata->label()->str() == "Detector Number") {
      countsShape[2] = static_cast<size_t>(dimMetadata->length());
    } else {
      throw std::runtime_error("Unexpected dimension label in histogram "
                               "message. Aborting deserialisation.");
    }
  }

  auto countsArray = messageData->data_as_ArrayUInt()->value();
  counts.resize(countsArray->size());
  std::copy(countsArray->begin(), countsArray->end(), counts.begin());
}