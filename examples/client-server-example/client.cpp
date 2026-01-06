#include <UnixLinx.h>
#include <LinxIpc.h>
#include <RawMessage.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

int main(int argc, char* argv[]) {
    std::cout << "===========================================\n";
    std::cout << "LinxIpc Client Application\n";
    std::cout << "===========================================\n\n";

    if (argc >= 3) {
        // Client mode: expects reqId and message size
        uint32_t reqId = std::atoi(argv[1]);
        size_t messageSize = std::atoi(argv[2]);
        std::string serverName = argc >= 4 ? argv[3] : "ExampleServer";

        std::cout << "Creating AfUnix client for server: " << serverName << "\n";
        std::cout << "Request ID: 0x" << std::hex << reqId << std::dec << "\n";
        std::cout << "Message size: " << messageSize << " bytes\n\n";

        auto client = AfUnixFactory::createClient(serverName);

        if (!client) {
            std::cerr << "Failed to create client\n";
            return -1;
        }

        if (!client->connect(5000)) {
            std::cerr << "Failed to connect to server\n";
            return -1;
        }

        // Create message with specified size filled with repeating digits 0-9
        std::vector<uint8_t> buffer(messageSize);
        for (size_t i = 0; i < messageSize; i++) {
            buffer[i] = '0' + (i % 10);
        }
        RawMessage message(reqId, buffer.data(), messageSize);

        std::cout << "Sending message...\n";
        client->send(message);

        std::cout << "Waiting for response (timeout: 5 seconds)...\n";

        // Wait for response using client's receive method
        auto response = client->receive(5000, LINX_ANY_SIG);

        if (response) {
            std::cout << "Response received!\n";
            std::cout << "  ReqId: 0x" << std::hex << response->getReqId() << std::dec << "\n";
            std::cout << "  Payload Size: " << response->getPayloadSize() << " bytes\n";

            bool reqIdMatch = response->getReqId() == reqId;
            bool sizeMatch = response->getPayloadSize() == message.getPayloadSize();
            bool payloadMatch = false;

            // Verify payload matches by comparing directly
            if (sizeMatch) {
                payloadMatch = (std::memcmp(message.getPayload(), response->getPayload(), messageSize) == 0);
                if (!payloadMatch) {
                    // Find first mismatch for debugging
                    const uint8_t* sentData = message.getPayload();
                    const uint8_t* responseData = response->getPayload();
                    for (size_t i = 0; i < messageSize; i++) {
                        if (responseData[i] != sentData[i]) {
                            std::cout << "  ✗ Payload mismatch at byte " << i
                                      << ": expected '" << (char)sentData[i] << "' (0x" << std::hex
                                      << (int)sentData[i] << "), got '" << (char)responseData[i]
                                      << "' (0x" << (int)responseData[i] << std::dec << ")\n";
                            break;
                        }
                    }
                }
            }

            if (reqIdMatch) {
                std::cout << "  ✓ ReqId matches!\n";
            } else {
                std::cout << "  ✗ ReqId mismatch! Expected: 0x" << std::hex << reqId << std::dec << "\n";
            }

            if (sizeMatch) {
                std::cout << "  ✓ Size matches!\n";
            } else {
                std::cout << "  ✗ Size mismatch! Expected: " << messageSize
                          << ", got: " << response->getSize() << "\n";
            }

            if (payloadMatch) {
                std::cout << "  ✓ Payload matches!\n";
            }

            if (reqIdMatch && sizeMatch && payloadMatch) {
                std::cout << "\n✓ Success: Message was echoed back correctly!\n";
            } else {
                std::cout << "\n✗ Verification failed!\n";
                return -1;
            }
        } else {
            std::cout << "\n✗ Timeout: No response received from server\n";
            return -1;
        }

    } else {
        // Info mode
        std::cout << "Echo client for LinxIpc server\n\n";
        std::cout << "Usage:\n";
        std::cout << "  " << (argc > 0 ? argv[0] : "./client_app") << " <reqId> <messageSize> [serverName]\n\n";
        std::cout << "Arguments:\n";
        std::cout << "  reqId        - Request ID (integer)\n";
        std::cout << "  messageSize  - Size of message payload in bytes\n";
        std::cout << "  serverName   - Server name (default: ExampleServer)\n\n";
        std::cout << "Example:\n";
        std::cout << "  " << (argc > 0 ? argv[0] : "./client_app") << " 42 1024\n";
        std::cout << "  " << (argc > 0 ? argv[0] : "./client_app") << " 100 512 ExampleServer\n";
    }

    std::cout << "===========================================\n";
    return 0;
}
