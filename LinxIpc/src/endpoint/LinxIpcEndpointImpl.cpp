#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocketImpl.h"
#include "LinxQueue.h"
#include "LinxTrace.h"
#include "LinxIpcPrivate.h"

LinxIpcEndpointImpl::LinxIpcEndpointImpl(LinxIpcSocket *socket) {
    assert(socket);
    this->socket = socket;
}


LinxIpcEndpointImpl::~LinxIpcEndpointImpl() {
    delete socket;
}

int LinxIpcEndpointImpl::send(const LinxMessageIpc &message, const std::string &to) {
    return socket->send(message, to);
}

int LinxIpcEndpointImpl::getPollFd() const {
    return socket->getFd();
}

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, 
                                               const std::vector<uint32_t> &sigsel, 
                                               const std::optional<std::string> &client) {

    LinxMessageIpcPtr msg{};
    std::string from;
    int ret = socket->receive(&msg, &from, timeoutMs);
    if (ret >= 0) {

        msg->setClient(std::make_shared<LinxIpcClientImpl>(shared_from_this(), from));
        auto predicate = [&sigsel, &from, &client](LinxMessageIpcPtr &msg) {
            if (!client || from == client.value()) {
                uint32_t reqId = msg->getReqId();
                return sigsel.size() == 0 || std::find_if(sigsel.begin(), sigsel.end(),
                                                        [reqId](uint32_t id) { return id == reqId; }) != sigsel.end();
            }

            return false;
        };

        return predicate(msg) ? msg : nullptr;
    }

    return nullptr;
}

std::string LinxIpcEndpointImpl::getName() const {
    return socket->getName();
}

LinxIpcEndpointPtr createLinxEndpoint(const std::string &serviceName) {
    auto socket = new LinxIpcSocketImpl(serviceName);
    if (socket->getFd() < 0) {
        delete socket;
        return nullptr;
    }

    return std::make_shared<LinxIpcEndpointImpl>(socket);
}