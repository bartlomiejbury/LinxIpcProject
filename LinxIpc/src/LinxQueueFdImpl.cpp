#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include "LinxQueueFdImpl.h"
#include "trace.h"

LinxQueueFdImpl::LinxQueueFdImpl() {
    if ((efd = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK)) < 0) {
        LOG_ERROR("Cannot open EventFD");
    }
}

LinxQueueFdImpl::~LinxQueueFdImpl() {
    if (efd >= 0) {
        close(efd);
    }
}

int LinxQueueFdImpl::writeEvent() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return -1;
    }

    uint64_t u = 1;
    if (int s = ::write(efd, &u, sizeof(uint64_t)); s != sizeof(uint64_t)) {
        LOG_ERROR("Write to EventFd failed: %d, errno: %d", s, errno);
        return -2;
    }

    return 0;
}

void LinxQueueFdImpl::clearEvents() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return;
    }

    uint64_t u = 0;
    int s = 0;
    while((s = ::read(efd, &u, sizeof(uint64_t))) == sizeof(uint64_t)) {}
}

int LinxQueueFdImpl::readEvent() {
    if (efd < 0) {
        LOG_ERROR("EventFD not opened");
        return -1;
    }

    uint64_t u = 0;
    if (int s = ::read(efd, &u, sizeof(uint64_t)); s != sizeof(uint64_t)) {
        LOG_ERROR("Read from EventFd failed: %d errno: %d", s, errno);
        return -2;
    }

    return 0;
}

int LinxQueueFdImpl::getFd() const {
    return efd;
}
