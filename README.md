## Linx Ipc Library for linux

Linx IPC based on AF_UNIX sockets:

- Message have unique Signal ID (uint32_t)
- Each enpoint have separate thread to receive messages
- Endpoint can be added to poll
- Filter messages by Signal ID or sender

### LinxIpcMessage
Each IPC message is identified by uint32_t request Id. Two requestID are reserved for internal purposes
```
#define IPC_HUNT_REQ 1
#define IPC_HUNT_RSP 2
```

LinxIpcMessage can be created in many ways:

IPC message with requestID = 10 and no payload:
```
auto msg = LinxMessageIpc(10);
```

IPC message with payload set to user struct:
```
struct Data {
    int a;
    char b;
} expectedMessage = {10, 5};
auto msg = LinxMessageIpc(10, expectedMessage);
```

Message payload can be read by template method ```getPayload<T>()```. This function will payload data to T* type
```
struct Data *data = msg.getPayload<struct Data>();
```
You can use  ```getPayload()``` to get raw data as uint8_t * typr
```
uint8_t *data = msg.getPayload<uint8_t>();
```

IPC message can be created from raw buffer:
```
uint8_t expectedMessage[] = {1, 2, 3, 3};
auto msg = LinxMessageIpc(10, expectedMessage, sizeof(expectedMessage));
```

Or by initializer list:
```
auto msg = LinxMessageIpc(10, {1, 2, 3});
```

Message payload length in bytes can be read by ```getPayloadSize()``` function

Message requestID can be read by ```getReqId()``` function

When message is received it has set sender client. It can be get by ```getCLient()``` method. This client can be used to identify sender os send back response. Locally created messages has set client to nullptr.


### LinxIpcEndpoint
LinxIpcEndpoint represents local endpoint of IPC comunication. Every application must have one endpoint. <br>
Endpoint is identified by unique name.<br>
Each enpoint will create internal thread to receive messages.

#### Create endpoint:
```
#include "LinxIpc.h"
auto endpoint = createLinxIpcServer("EndointName", 100);
endpoint->start();
```

Second parameter indicates queue size. If queue is empty new messages are dropped.

You can check current queue occupancy by
```
endpint->getQueueSize();
```
#### Receive message:
```
auto endpoint = createLinxIpcServer("EndointName", 100);
endpoint->start();
...
auto msg = endpoint->receive(IMMEDIATE_TIMEOUT, LINX_ANY_SIG, LINX_ANY_FROM);
```

First parameter indicates how long in miliseconds receive function will wait for message that matches filter.
If time expire receive function return nullptr.
```LINX_ANY_SIG``` - receive any signal

Second parameter indicates which signals are expected to receive.
Others signals are added to queue but not interrupt receive waiting.
Third parameter indicates which sender are expected to receive message from.
Signals from other sources not interrupt receive waiting.
```LINX_ANY_FROM```- receive signal from any source

You can create client by calling createClient method on endpoint. LinxIpcClient represent remote endpoint
```
auto endpoint = createLinxIpcServer("EndointName", 100);
auto client = endpoint->createClient("ClientName");
endpoint->start();
```

LinxIpcClient can be also used to receive message from this client.
```
auto endpoint = createLinxIpcServer("EndointName", 100);
auto client = endpoint->createClient("ClientName");
endpoint->start();
auto msg = client->receive(10000, {20});
```

#### Send message:

```
auto endpoint = createLinxIpcServer("EndointName", 100);
auto client = endpoint->createClient("ClientName");
endpoint->start();

LinxMessageIpc message1{12};
client->send(&message1);
```

Message can be send directly by endpoint
```
LinxMessageIpc message2{20};
endpoint->send(&message2, client);
```

#### Connecting to endpoint:
There is no guarantee remote endpoint is alive. To check remote endpoint is alive call ```connect``` function on client
```
auto endpoint = createLinxIpcServer("EndointName", 100);
auto client = endpoint->createClient("ClientName");
endpoint->start();

if (client->connect(5000)) {
    // client is alive
}
```

This function will send HUNT_REQ messages to client every 500 ms. When HUNT_RSP message is received, client is alive and connect returns true.

Passing ```IMMEDIATE_TIMEOUT``` to connect will cause one ping to be send.

#### Pooling endpoint:
You can register message handlers in LinxIpcEndpoint. Calling receive method without any arguments will get first message from queue and execute registered callback. This function will never block. Return value from callbeck is returned by receive() call.
If queue is empty receive() call will return -1

You can add LinxIpcENdpoint to poll by getting it file descriptor by ```getFd()``` function.

```
int callback(LinxMessageIpc *msg, void *data) {
    printf("received message from: %s\n", msg->getClient()->getName().c_str());
    return 0;
}

auto endpoint = createLinxIpcServer("EndointName", 100);
endpoint->registerCallback(13, callback, nullptr);
endpoint->start();
```
