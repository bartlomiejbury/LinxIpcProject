#include "AfUnix.h"
#include "common/LinxMessage.h"
#include <iostream>
#include <thread>

// Simple AF_UNIX communication example

void serverThread() {
    auto server = AfUnixFactory::createServer("ExampleServer", 10);
    if (!server) {
        std::cerr << "Failed to create server" << std::endl;
        return;
    }

    std::cout << "Server started, waiting for messages..." << std::endl;

    // Receive message
    auto msg = server->receive(5000, LINX_ANY_SIG);
    if (msg && msg->message) {
        std::cout << "Server received message with signal: " << msg->message->getReqId() << std::endl;
    }
}

void clientThread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = AfUnixFactory::createClient("ExampleServer");
    if (!client) {
        std::cerr << "Failed to create client" << std::endl;
        return;
    }

    std::cout << "Client connected, sending message..." << std::endl;

    // Send message
    RawMessage msg(42);
    client->send(msg);

    std::cout << "Client sent message" << std::endl;
}

int main() {
    std::thread server(serverThread);
    std::thread client(clientThread);

    server.join();
    client.join();

    return 0;
}
