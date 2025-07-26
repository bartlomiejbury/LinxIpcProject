
#include "LinxIpc.h"

int main() {

    auto handler = LinxIpcHandlerBuilder::Simple("TEST1").build();
    auto client = handler->createClient("TEST3");
    auto server = handler->getServer();

    auto msg = client->receive(10000, {20});
    if (msg) {
        printf("error: received msg: %d\n", msg->getReqId());
    } else {
        printf("OK: not received any message\n");
    }

    while (1) {
        auto msg = server->receive(INFINITE_TIMEOUT, {});
        if (msg) {
            printf("OK: received msg: %d from %s\n", msg->getReqId(), msg->getClient()->getName().c_str());
        } else {
            printf("OK: not received any message\n");
        }
    }
}
