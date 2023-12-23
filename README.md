## Linx Ipc Library for linux

Linx IPC based on AF_UNIX sockets:

- Message have unique Signal ID (uint32_t)
- Each enpoint have separate thread to receive messages
- Endpoint can be added to poll
- Filter messages by Signal ID or sender

### LinxIpcMessage
TODO:

### LinxIpcEndpoint
LinxIpcEndpoint represents local endpoint of IPC comunication. Every application must have one endpoint. <br>
Endpoint is identified by unique name.<br>
Each enpoint will create internal thread to receive messages.

#### Create endpoint:
```
#include "LinxIpc.h"
auto endpoint = createLinxIpcEndpoint("EndointName", 100);
```

Second parameter indicates queue size. If queue is empty new messages are dropped.

You can check current queue occupancy by
```
endpint->getQueueSize();
```
#### Receive message:
```
auto endpoint = createLinxIpcEndpoint("EndointName", 100);
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
auto client = endpoint->createClient("ClientName");
```

LinxIpcClient can be also used to receive message from this client.
```
auto endpoint = createLinxIpcEndpoint("EndointName", 100);
auto client = endpoint->createClient("ClientName");
auto msg = client->receive(10000, {20});
```

#### Send message:

```
auto endpoint = createLinxIpcEndpoint("EndointName", 100);
auto client = endpoint->createClient("ClientName");

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
auto endpoint = createLinxIpcEndpoint("EndointName", 100);
auto client = endpoint->createClient("ClientName");
if (client->connect(5000)) {
    // client is alive
}
```

This function will send PING_REQ messages to client every 500 ms. When PING_RSP message is received, client is alive and connect returns true.

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

auto endpoint = createLinxIpcEndpoint("EndointName", 100);
endpoint->registerCallback(13, callback, nullptr);
```
