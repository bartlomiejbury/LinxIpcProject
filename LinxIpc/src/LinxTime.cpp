#include "LinxTime.h"

uint64_t getTimeMs() {

    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint64_t milisecondsFromSeconds = ts.tv_sec * MILLI_SECONDS;
    uint64_t milisecondsFromNanoseconds = ts.tv_nsec / (NANO_SECONDS / MILLI_SECONDS);
    return milisecondsFromSeconds + milisecondsFromNanoseconds;
}

timespec timeMsToTimespec(uint64_t timeMs) {
    timespec ts{};
    ts.tv_sec = timeMs / MILLI_SECONDS;
    ts.tv_nsec = (timeMs % MILLI_SECONDS) * (NANO_SECONDS / MILLI_SECONDS);
    return ts;
}