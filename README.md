# LinxIpc Library for Linux

A high-performance IPC (Inter-Process Communication) library for Linux based on AF_UNIX sockets.

## Features

- **Message-based communication** with unique Signal IDs (uint32_t)
- **Separate thread** for receiving messages per endpoint
- **Poll support** for integrating with event loops
- **Message filtering** by Signal ID or sender
- **Thread-safe message queues** with configurable size
- **Type-safe message payloads** using C++ templates
- **Connection verification** with ping/pong mechanism

## Architecture

### Key Components

- **LinxMessage**: Message class with typed payload support
- **AfUnixServer**: Server endpoint that can receive messages from multiple clients
- **AfUnixClient**: Client endpoint for sending messages to servers
- **LinxIpcHandler**: Message dispatcher with callback registration

## Message API

### Creating Messages

LinxMessage is identified by a `uint32_t` request ID.

**Empty message:**
```cpp
LinxMessage msg(20);
```

**Message with typed payload:**
```cpp
struct Data {
    int value;
    char flag;
};

Data payload = {42, 'A'};
LinxMessage msg(20, payload);
```

**Message from raw buffer:**
```cpp
uint8_t buffer[] = {1, 2, 3, 4};
LinxMessage msg(20, buffer, sizeof(buffer));
```

**Message from vector:**
```cpp
std::vector<uint8_t> data = {1, 2, 3};
LinxMessage msg(20, data);
```

**Deserialize message:**
```cpp
uint8_t data[] = {1, 2, 3};
LinxMessage msg(data, sizeof(data));
```

### Reading Message Payload

**Get typed payload:**
```cpp
Data *payload = msg.getPayload<Data>();
printf("Value: %d\n", payload->value);
```

**Get raw payload:**
```cpp
uint8_t *raw = msg.getPayload();
```

**Get payload size:**
```cpp
uint32_t size = msg.getPayloadSize();
```

**Get request ID:**
```cpp
uint32_t reqId = msg.getReqId();
```

## Server API

### Creating a Server

```cpp
#include "AfUnixServer.h"
#include "LinxIpc.h"

// Create server with default queue size (100)
auto server = AfUnixServer::create("MyServer");

// Or specify custom queue size
auto server = AfUnixServer::create("MyServer", 200);

// Start the server thread
server->start();
```

### Receiving Messages

**Receive any message:**
```cpp
auto msg = server->receive(INFINITE_TIMEOUT);
if (msg) {
    printf("Received: 0x%x from %s\n",
           msg->message->getReqId(),
           msg->context->getName().c_str());
}
```

**Receive with timeout (milliseconds):**
```cpp
auto msg = server->receive(5000);  // Wait up to 5 seconds
if (!msg) {
    printf("Timeout\n");
}
```

**Receive specific message types:**
```cpp
// Wait for message with reqId 20 or 30
auto msg = server->receive(INFINITE_TIMEOUT, {20, 30});
```

**Receive from specific client:**
```cpp
auto client = server->createContext("ClientName");
auto msg = server->receive(INFINITE_TIMEOUT, LINX_ANY_SIG, client);
```

### Creating Contexts

Create a context to represent a remote client:

```cpp
auto client = server->createContext("RemoteClient");

// Use context to receive messages only from that client
auto msg = client->receive(1000, {20});
```

### Using Message Handlers

Register callbacks for specific message types:

```cpp
#include "LinxIpc.h"

auto server = AfUnixServer::create("MyServer");
auto handler = LinxIpcHandler(server);

// Register callback for message ID 20
handler.registerCallback(20,
    [](const LinxReceivedMessageSharedPtr &msg, void *data) {
        printf("Received: 0x%x from %s\n",
               msg->message->getReqId(),
               msg->context->getName().c_str());
        return 0;  // Return value can be checked
    },
    nullptr);  // Optional user data

handler.start();

// Process messages in loop
while (true) {
    handler.handleMessage(1000);  // Process one message
}
```

### Polling Support

Integrate server with poll/select:

```cpp
#include <poll.h>

auto server = AfUnixServer::create("MyServer");
server->start();

struct pollfd fds[1];
fds[0].fd = server->getPollFd();
fds[0].events = POLLIN;

while (true) {
    int rc = poll(fds, 1, 10000);
    if (rc > 0) {
        auto msg = server->receive(IMMEDIATE_TIMEOUT);
        if (msg) {
            printf("Received: 0x%x\n", msg->message->getReqId());
        }
    }
}
```

## Client API

### Creating a Client

```cpp
#include "AfUnixClient.h"
#include "LinxIpc.h"

auto client = AfUnixClient::create("MyClientName");
```

### Connecting to Server

Verify the server is alive before sending:

```cpp
if (client->connect(5000)) {  // 5 second timeout
    printf("Connected to server\n");
} else {
    printf("Failed to connect\n");
}
```

### Sending Messages

```cpp
LinxMessage msg(20);
int rc = client->send(msg);
if (rc < 0) {
    printf("Send failed\n");
}
```

### Send and Receive

Send a message and wait for response:

```cpp
LinxMessage request(10);
auto response = client->sendReceive(request, 5000, {20});
if (response) {
    printf("Got response: 0x%x\n", response->getReqId());
}
```

## Constants

```cpp
INFINITE_TIMEOUT   // -1, wait forever
IMMEDIATE_TIMEOUT  // 0, don't wait
LINX_ANY_SIG      // {}, receive any message type
LINX_ANY_FROM     // nullptr, receive from any sender
LINX_DEFAULT_QUEUE_SIZE  // 100
```

## Complete Example

**Server (receiver.cpp):**
```cpp
#include <stdio.h>
#include "AfUnixServer.h"
#include "LinxIpc.h"

int main() {
    auto server = AfUnixServer::create("MyServer");
    auto handler = LinxIpcHandler(server);

    handler.registerCallback(20,
        [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            printf("Received: 0x%x from %s\n",
                   msg->message->getReqId(),
                   msg->context->getName().c_str());
            return 0;
        }, nullptr);

    handler.start();

    while (true) {
        handler.handleMessage(1000);
    }

    return 0;
}
```

**Client (sender.cpp):**
```cpp
#include <stdio.h>
#include "AfUnixClient.h"
#include "LinxIpc.h"

int main() {
    auto client = AfUnixClient::create("MySender");

    if (!client->connect(5000)) {
        printf("Failed to connect\n");
        return -1;
    }

    LinxMessage msg(20);
    if (client->send(msg) < 0) {
        printf("Failed to send\n");
        return -1;
    }

    printf("Message sent successfully\n");
    return 0;
}
```

## Logging

LinxIpc includes a built-in logging system that can be configured at compile-time and runtime.

### Compile-Time Configuration

Set the `USE_LOGGING` level when configuring CMake:

```bash
# No logging (default if not specified)
cmake -B build

# Error level only
cmake -B build -DUSE_LOGGING=1

# Warning and error
cmake -B build -DUSE_LOGGING=2

# Info, warning, and error
cmake -B build -DUSE_LOGGING=3

# All levels including debug
cmake -B build -DUSE_LOGGING=4
```

**Logging Levels:**
- `1` - `SEVERITY_ERROR`: Critical errors only
- `2` - `SEVERITY_WARNING`: Warnings and errors
- `3` - `SEVERITY_INFO`: Informational messages, warnings, and errors
- `4` - `SEVERITY_DEBUG`: All messages including debug output

### Runtime Configuration

Override the log level at runtime using the `LOG_LEVEL` environment variable:

```bash
# Set log level to INFO for this run
LOG_LEVEL=3 ./myapp

# Set log level to DEBUG
LOG_LEVEL=4 ./myapp
```

### Using Logging Macros

Initialize logging in your application:

```cpp
#include "trace.h"

int main() {
    TRACE_INIT();  // Initialize logging system

    TRACE_ERROR("Critical error: %d", errorCode);
    TRACE_WARNING("Warning condition detected");
    TRACE_INFO("Server started on port %d", port);
    TRACE_DEBUG("Processing message ID: %d", msgId);

    return 0;
}
```

**Available Macros:**
- `TRACE_INIT()` - Initialize logging (call once at startup)
- `TRACE_ERROR(...)` - Log error messages
- `TRACE_WARNING(...)` - Log warnings
- `TRACE_INFO(...)` - Log informational messages
- `TRACE_DEBUG(...)` - Log debug messages
- `TRACE_ENTER()` - Log function entry (debug level)
- `TRACE_EXIT()` - Log function exit (debug level)

Logging output is sent to syslog and can be viewed with:

```bash
journalctl -f
```

### Custom Logging Implementation

You can replace the default logging implementation by setting `LINX_LOG_PREFIX` to your custom function prefix:

```cmake
# In your CMakeLists.txt
set(LINX_LOG_PREFIX mylog)
```

Then implement these functions with your custom logging:

```cpp
extern "C" {
    void mylog_error(const char *fileName, int lineNum, const char *format, ...);
    void mylog_warning(const char *fileName, int lineNum, const char *format, ...);
    void mylog_info(const char *fileName, int lineNum, const char *format, ...);
    void mylog_debug(const char *fileName, int lineNum, const char *format, ...);
}
```

The library will call your functions instead of the default `trace_*` functions. This allows you to integrate with custom logging frameworks, file logging, or other output destinations.

**Example custom logger:**
```cpp
#include <cstdio>
#include <cstdarg>

extern "C" void mylog_info(const char *fileName, int lineNum,
                           const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("[INFO] %s:%d: ", fileName, lineNum);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}
// Implement other levels similarly...
```

## Building

```bash
cmake -B build
cmake --build build
```

## Testing

```bash
cmake -B build_ut -DCMAKE_BUILD_TYPE=Debug
cmake --build build_ut
cd build_ut && ctest
```

## Thread Safety

- Server and Client objects are thread-safe for concurrent operations
- Message queues are protected with mutexes
- Each server runs its own receive thread

## License

See LICENSE.txt for details.
