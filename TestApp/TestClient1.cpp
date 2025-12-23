#include <stdio.h>
#include "AfUnixServer.h"
#include "LinxIpc.h"

int main() {

    auto server = AfUnixServer::create("TEST3");
    auto client = server->createContext("TEST3");

    auto msg = client->receive(10000, {20});
    if (msg) {
        printf("error: received msg: %d\n", msg->getReqId());
    } else {
        printf("OK: not received any message\n");
    }

    while (1) {
        auto msg = server->receive(INFINITE_TIMEOUT);
        if (msg) {
            printf("OK: received msg: 0x%x from %s\n", msg->message->getReqId(), msg->context->getName().c_str());
        } else {
            printf("OK: not received any message\n");
        }
    }
}
