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
      Array::ArrayFloat, builder.CreateVector(timeOfFlight).Union());

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

  auto histogramDataOffset = CreateEventHistogram(
      builder, builder.CreateString("NeXus-Streamer"), timestampUnix,
      builder.CreateVector(dimensionsArray), timestampUnix,
      builder.CreateVector(countsShapeUInt), builder.CreateVector(offsets),
      Array::ArrayUInt, builder.CreateVector(counts).Union());

  FinishEventHistogramBuffer(builder, histogramDataOffset);

  return Streamer::Message(builder.Release());
}
