#include <AfUnix.h>
#include <LinxIpc.h>
#include <RawMessage.h>
#include <iostream>

int main() {
    std::cout << "===========================================\n";
    std::cout << "LinxIpc Server Application\n";
    std::cout << "===========================================\n\n";

    std::cout << "Creating AfUnix server 'ExampleServer'...\n";
    auto server = AfUnixFactory::createServer("ExampleServer", 10);

    if (!server) {
        std::cerr << "Failed to create server\n";
        return -1;
    }

    auto handler = LinxIpcHandler(server);

    // Register callback for signal 42
    handler.registerCallback(42, [](const LinxReceivedMessageSharedPtr &msg, void *data) {
        std::cout << "Received message with reqId: 0x" << std::hex << msg->message->getReqId()
                  << " from: " << msg->from->format() << std::endl;
        return 0;
    }, nullptr);

    std::cout << "Server started, waiting for messages...\n";
    std::cout << "Press Ctrl+C to stop\n\n";

    handler.start();

    while (true) {
        handler.handleMessage(1000);
    }

    return 0;
}
