#include <AfUnix.h>
#include <UdpLinx.h>
#include <RawMessage.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::cout << "===========================================\n";
    std::cout << "LinxIpc Client Application\n";
    std::cout << "===========================================\n\n";

    if (argc >= 2) {
        // Example: AfUnix client mode
        std::string serverName = argv[1];
        uint32_t messageValue = argc >= 3 ? std::atoi(argv[2]) : 42;

        std::cout << "Creating AfUnix client for server: " << serverName << "\n";
        auto client = AfUnixFactory::createClient(serverName);

        if (!client) {
            std::cerr << "Failed to create client\n";
            return -1;
        }

        if (!client->connect(5000)) {
            std::cerr << "Failed to connect to server\n";
            return -1;
        }

        std::cout << "Sending message: " << messageValue << "\n";
        RawMessage message(messageValue);
        client->send(message);

        std::cout << "Message sent successfully!\n";
    } else {
        // Info mode
        std::cout << "Available IPC mechanisms:\n";
        std::cout << "  - AF_UNIX (Unix Domain Sockets)\n";
        std::cout << "    Types: AfUnixClient, AfUnixServer\n";
        std::cout << "  - UDP (User Datagram Protocol)\n";
        std::cout << "    Types: UdpClient, UdpServer\n\n";

        std::cout << "Library successfully linked via CMake Package Config!\n\n";
        std::cout << "Usage:\n";
        std::cout << "  " << (argc > 0 ? argv[0] : "./client_app") << "                    - Show this info\n";
        std::cout << "  " << (argc > 0 ? argv[0] : "./client_app") << " <server> <value>  - Send message to server\n";
    }

    std::cout << "===========================================\n";
    return 0;
}
