
#include "LinxIpc.h"

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <server> <message>\n", argv[0]);
        return 1;
    }

    std::string serverName = argv[1];
    uint32_t messageValue = std::atoi(argv[2]);

    auto handler = LinxIpcHandlerBuilder::Simple("ipcSender").build();

    auto client = handler->createClient(serverName);
    if (!client->connect(5000)) {
        printf("Failed to connect client to server\n");
        return -1;
    }

    LinxMessageIpc message1{messageValue};
    int rc = client->send(message1);
    if (rc < 0) {
        printf("Failed to send message: %d\n", rc);
        return -1;
    }
}
