#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>
#include <cstdint>
#include "LinxIpcEventFdImpl.h"
#include "LinxTrace.h"

LinxIpcEventFdImpl::LinxIpcEventFdImpl() {
    if ((efd = eventfd(0, EFD_SEMAPHORE | EFD_NONBLOCK)) < 0) {
        LINX_ERROR("Cannot open EventFD");
    }
}

LinxIpcEventFdImpl::~LinxIpcEventFdImpl() {
    if (efd >= 0) {
        close(efd);
    }
}

int LinxIpcEventFdImpl::writeEvent() {
    if (efd < 0) {
        LINX_ERROR("EventFD not opened");
        return -1;
    }

    uint64_t u = 1;
    if (int s = ::write(efd, &u, sizeof(uint64_t)); s != sizeof(uint64_t)) {
        LINX_ERROR("Write to EventFd failed: %d, errno: %d", s, errno);
        return -2;
    }

    return 0;
}

void LinxIpcEventFdImpl::clearEvents() {
    if (efd < 0) {
        LINX_ERROR("EventFD not opened");
        return;
    }

    uint64_t u = 0;
    int s = 0;
    while((s = ::read(efd, &u, sizeof(uint64_t))) == sizeof(uint64_t)) {}
}

int LinxIpcEventFdImpl::readEvent() {
    if (efd < 0) {
        LINX_ERROR("EventFD not opened");
        return -1;
    }

    uint64_t u = 0;
    if (int s = ::read(efd, &u, sizeof(uint64_t)); s != sizeof(uint64_t)) {
        LINX_ERROR("Read from EventFd failed: %d errno: %d", s, errno);
        return -2;
    }

    return 0;
}

int LinxIpcEventFdImpl::getFd() const {
    return efd;
}
