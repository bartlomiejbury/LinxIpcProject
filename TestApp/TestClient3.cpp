
#include <stdio.h>
#include "AfUnixServer.h"
#include "LinxIpc.h"

int main() {

    auto server = AfUnixFactory::createServer("TEST1");
    auto handler = LinxIpcHandler(server);
    handler.registerCallback(20, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            printf("Received request: 0x%x from %s\n", msg->message->getReqId(), msg->context->getName().c_str());
            return 0;
        }, nullptr);

    handler.start();
    while (1) {
        handler.handleMessage(1000);
    }
}
