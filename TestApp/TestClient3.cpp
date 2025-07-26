
#include "LinxIpc.h"

int main() {

    auto handler = LinxIpcHandlerBuilder::Extended("TEST1")
        .registerCallback(20, [](LinxMessageIpc *msg, void *data) {
            printf("Received request: %d from: %s\n", msg->getReqId(), msg->getClient()->getName().c_str());
            return 0;
        }, nullptr)
        .build();

    while (1) {
        handler->handleMessage(1000);
    }
}
