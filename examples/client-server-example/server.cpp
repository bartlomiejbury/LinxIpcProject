#include <UnixLinx.h>
#include <LinxIpc.h>
#include <RawMessage.h>
#include <iostream>

int main() {
    std::cout << "===========================================\n";
    std::cout << "LinxIpc Echo Server Application\n";
    std::cout << "===========================================\n\n";

    std::cout << "Creating AfUnix server 'ExampleServer'...\n";
    auto server = AfUnixFactory::createServer("ExampleServer", 10);
    if (!server) {
        std::cerr << "Failed to create server\n";
        return -1;
    }

    std::cout << "Echo server started, waiting for messages...\n";
    std::cout << "Press Ctrl+C to stop\n\n";

    server->start();
    while (true) {
        // Receive message with 1 second timeout
        auto receivedMsg = server->receive(1000);

        if (receivedMsg) {
            std::cout << "Received message with reqId: 0x" << std::hex << receivedMsg->message->getReqId() << std::dec
                      << ", size: " << receivedMsg->message->getSize()
                      << " bytes from: " << receivedMsg->from->format() << std::endl;

            // Echo the message back to sender
            std::cout << "Echoing message back to sender..." << std::endl;
            server->send(*receivedMsg->message, *receivedMsg->from);
        }
    }

    return 0;
}
