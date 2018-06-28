#include "EventData.h"
#include "benchmark/benchmark.h"

void SerialiseData(benchmark::State &state) {
  // Generate fake event data of consecutive numbers
  size_t numberOfFakeEvents = 1000;
  std::vector<uint32_t> detIds(numberOfFakeEvents);
  std::vector<uint32_t> tofs(numberOfFakeEvents);
  int n = 0;
  std::generate(detIds.begin(), detIds.end(), [n]() mutable { return n++; });
  std::generate(tofs.begin(), tofs.end(), [n]() mutable { return n++; });
  float protonCharge = 0.001142;
  uint64_t frameTime = 41389;
  uint32_t period = 1;

  // Populate an EventData object
  auto events = EventData();
  events.setDetId(detIds);
  events.setTof(tofs);
  events.setProtonCharge(protonCharge);
  events.setFrameTime(frameTime);
  events.setPeriod(period);

  std::string rawbuf;

  // Benchmark just the serialisation process
  while (state.KeepRunning()) {
    events.getBufferPointer(rawbuf, 0);
  }
}

BENCHMARK(SerialiseData)->Repetitions(10);

BENCHMARK_MAIN();
