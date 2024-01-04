#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include "LinxQueueFd.h"
#include "trace.h"

LinxQueueFd::LinxQueueFd() {
    if ((efd = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK)) < 0) {
        LOG_ERROR("Cannot open EventFD");
    }
}

LinxQueueFd::~LinxQueueFd() {
    if (efd > 0) {
        close(efd);
    }
}

void LinxQueueFd::writeEvent() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return;
    }

    uint64_t u = 1;
    int s = ::write(efd, &u, sizeof(uint64_t));
    if (s != sizeof(uint64_t)) {
        LOG_ERROR("Write to EventFd failed: %d, errno: %d", s, errno);
    }
}

void LinxQueueFd::clearEvents() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return;
    }

    uint64_t u = 0;
    int s = 0;
    while((s = ::read(efd, &u, sizeof(uint64_t))) != -1) {}
}

void LinxQueueFd::readEvent() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return;
    }

    uint64_t u = 0;
    int s = ::read(efd, &u, sizeof(uint64_t));
    if (s != sizeof(uint64_t)) {
        LOG_ERROR("Read from EventFd failed: %d errno: %d", s, errno);
    }
}

int LinxQueueFd::getFd() const {
    return efd;
}
