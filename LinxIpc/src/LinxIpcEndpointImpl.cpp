#include <cassert>
#include "LinxIpc.h"
#include "LinxIpcEndpointImpl.h"
#include "LinxIpcClientImpl.h"
#include "LinxIpcSocket.h"
#include "LinxQueue.h"
#include "trace.h"

LinxIpcEndpointImpl::LinxIpcEndpointImpl(LinxIpcSocket *socket) {
    assert(socket);;
    this->socket = socket;
}

LinxIpcEndpointImpl::~LinxIpcEndpointImpl() {
    delete socket;
}

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel,
                                               const LinxIpcClientPtr &client) {

    LinxMessageIpcPtr msg{};
    std::string from;

    int ret = socket->receive(&msg, &from, timeoutMs);
    if (ret >= 0) {

        msg->setClient(createClient(from));
        LOG_DEBUG("Received request on IPC: %s: %d from: %s", socket->getName().c_str(), msg->getReqId(),
                msg->getClient()->getName().c_str());

        auto predicate = [&sigsel, &from, &client](std::shared_ptr<LinxMessageIpc> &msg) {
            if (!client || from == client->getName()) {
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

int LinxIpcEndpointImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    if (to == nullptr) {
        return -1;
    }
    return this->socket->send(message, to->getName());
}

LinxIpcClientPtr LinxIpcEndpointImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

int LinxIpcEndpointImpl::getPollFd() const {
    return socket->getFd();
}
