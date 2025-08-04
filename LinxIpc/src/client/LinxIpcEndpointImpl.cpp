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

int LinxIpcEndpointImpl::send(const LinxMessageIpc &message, const LinxIpcClientPtr &to) {
    return socket->send(message, to->getName());
}

int LinxIpcEndpointImpl::getPollFd() const {
    return socket->getFd();
}

LinxMessageIpcPtr LinxIpcEndpointImpl::receive(int timeoutMs, const std::vector<uint32_t> &sigsel, const LinxIpcClientPtr &client) {

    LinxMessageIpcPtr msg{};
    std::string from;
    int ret = socket->receive(&msg, &from, timeoutMs);
    if (ret >= 0) {

        msg->setClient(createClient(from));
        auto predicate = [&sigsel, &from, &client](LinxMessageIpcPtr &msg) {
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

LinxIpcClientPtr LinxIpcEndpointImpl::createClient(const std::string &serviceName) {
    return std::make_shared<LinxIpcClientImpl>(shared_from_this(), serviceName);
}

void LinxIpcEndpointImpl::start() {}
void LinxIpcEndpointImpl::stop() {}