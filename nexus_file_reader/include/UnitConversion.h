#pragma once

#include <cstdint>
#include <vector>

uint64_t secondsToNanoseconds(double seconds);

uint64_t secondsToMilliseconds(double seconds);

uint64_t nanosecondsToMilliseconds(uint64_t nanoseconds);

std::vector<uint64_t> secondsToNanoseconds(std::vector<double> const &seconds);
