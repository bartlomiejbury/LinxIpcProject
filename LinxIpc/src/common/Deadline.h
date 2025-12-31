#pragma once

#include <chrono>
#include "LinxIpc.h"

class Deadline {
    public:
        Deadline(int timeoutMs) {
            if (timeoutMs != INFINITE_TIMEOUT) {
                deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
            } else {
                deadline = std::chrono::steady_clock::time_point::max();
            }
        }

        bool isExpired() const {
            if (deadline == std::chrono::steady_clock::time_point::max()) {
                return false;
            }
            return std::chrono::steady_clock::now() > deadline;
        }

        int getRemainingTimeMs() const {
            if (deadline == std::chrono::steady_clock::time_point::max()) {
                return INFINITE_TIMEOUT;
            }
            auto now = std::chrono::steady_clock::now();
            if (now >= deadline) {
                return 0;
            }
            return std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();
        }

    private:
        std::chrono::steady_clock::time_point deadline;
};