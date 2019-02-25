#include "HistogramData.h"
#include "../../core/include/HistogramFrame.h"
#include "../../core/include/Message.h"
#include "hs00_event_histogram_generated.h"

Streamer::Message createHistogramMessage(const HistogramFrame &histogram,
                                         uint64_t timestampUnix) {
  flatbuffers::FlatBufferBuilder builder;

  auto periodsDimension = CreateDimensionMetaData(
      builder, static_cast<uint32_t>(histogram.countsShape[0]), 0,
      builder.CreateString("Period"));

  auto timeOfFlightDimension = CreateDimensionMetaData(
      builder, static_cast<uint32_t>(histogram.countsShape[1]),
      builder.CreateString("us"), builder.CreateString("Time Of Flight"),
      Array::ArrayFloat,
      CreateArrayFloat(builder, builder.CreateVector(histogram.timeOfFlight))
          .Union());

  std::vector<uint32_t> detIDs(histogram.detectorIDs.cbegin(),
                               histogram.detectorIDs.cend());

  auto detectorsDimension = CreateDimensionMetaData(
      builder, static_cast<uint32_t>(detIDs.size()), 0,
      builder.CreateString("Detector Number"), Array::ArrayUInt,
      CreateArrayUInt(builder, builder.CreateVector(detIDs)).Union());

  std::vector<flatbuffers::Offset<DimensionMetaData>> dimensionsArray{
      periodsDimension, timeOfFlightDimension, detectorsDimension};

  const std::vector<uint32_t> countsShapeUInt{
      static_cast<uint32_t>(histogram.countsShape[0]),
      static_cast<uint32_t>(histogram.countsShape[1]),
      static_cast<uint32_t>(histogram.countsShape[2])};

  const std::vector<uint32_t> offsets{0, 0, 0};

  std::vector<uint32_t> countsUInt(histogram.counts.cbegin(),
                                   histogram.counts.cend());

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
 * @param message - input message to be deserialised
 * @param timestampUnix - output by reference
 */
HistogramFrame deserialiseHistogramMessage(Streamer::Message &message,
                                           uint64_t &timestampUnix) {
  auto messageData = GetEventHistogram(message.data());
  timestampUnix = messageData->timestamp();

  std::vector<size_t> countsShape(3);
  std::vector<float> timeOfFlight;
  std::vector<int32_t> detIds;
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
      auto detIDsArray = messageData->data_as_ArrayUInt()->value();
      detIds.resize(detIDsArray->size());
      std::copy(detIDsArray->begin(), detIDsArray->end(), detIds.begin());
    } else {
      throw std::runtime_error("Unexpected dimension label in histogram "
                               "message. Aborting deserialisation.");
    }
  }

  auto countsArray = messageData->data_as_ArrayUInt()->value();
  std::vector<int32_t> counts(countsArray->size());
  std::copy(countsArray->begin(), countsArray->end(), counts.begin());

  return HistogramFrame(counts, countsShape, timeOfFlight, detIds);
}
