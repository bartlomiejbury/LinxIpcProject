#pragma once

#include <cstdint>
#include <time.h>

constexpr uint64_t NANO_SECONDS = 1000000000;
constexpr uint64_t MILLI_SECONDS = 1000;

uint64_t getTimeMs();
timespec timeMsToTimespec(uint64_t timeMs);
