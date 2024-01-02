
#include "LinxIpc.h"

int main() {

    auto endpoint = createLinxIpcServer("TEST2");
    endpoint->start();
    auto client = endpoint->createClient("TEST1");

    client->connect(5000);

    LinxMessageIpc message1{12};
    client->send(message1);

    LinxMessageIpc message2{20};
    endpoint->send(message2, client);
}
