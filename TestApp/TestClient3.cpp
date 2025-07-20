
#include "LinxIpc.h"

int main() {

    LinxIpcHandlerBuilder builder("TEST1");
    auto handler = builder.setExtendedServer(100).build();

    handler->registerCallback(20, [](LinxMessageIpc *msg, void *data) {
            printf("Received request: %d from: %s\n", msg->getReqId(), msg->getClient()->getName().c_str());
            return 0;
        }, nullptr);

    while (1) {
        handler->handleMessage(1000);
    }
}
