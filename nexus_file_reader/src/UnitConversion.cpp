#include <algorithm>
#include <cmath>

#include "UnitConversion.h"

uint64_t secondsToNanoseconds(double seconds) {
  return static_cast<uint64_t>(round(seconds * 1000000000));
}

uint64_t secondsToMilliseconds(double seconds) {
  return static_cast<uint64_t>(round(seconds * 1000));
}

uint64_t nanosecondsToMilliseconds(uint64_t nanoseconds) {
  return nanoseconds / 1000000ULL;
}

std::vector<uint64_t> secondsToNanoseconds(std::vector<double> const &seconds) {
  std::vector<uint64_t> nanoseconds;
  std::transform(seconds.cbegin(), seconds.cend(),
                 std::back_inserter(nanoseconds),
                 [](double const secondsValue) {
                   return secondsToNanoseconds(secondsValue);
                 });
  return nanoseconds;
}
