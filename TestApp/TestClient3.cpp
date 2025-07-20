
#include "LinxIpc.h"

int main() {

    auto endpoint = createLinxIpcServer("TEST1");
    auto handler = createLinxIpcHandler(endpoint);

    endpoint->start();

    handler->registerCallback(20, [](LinxMessageIpc *msg, void *data) {
            printf("Received request: %d from: %s\n", msg->getReqId(), msg->getClient()->getName().c_str());
            return 0;
        }, nullptr);

    while (1) {
        handler->handleMessage(1000);
    }
}
