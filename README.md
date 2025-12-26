# LinxIpc Library for Linux

A high-performance IPC (Inter-Process Communication) library for Linux supporting both Unix Domain Sockets and UDP.

## Features

- **Multiple transport protocols**: AF_UNIX sockets and UDP (including multicast)
- **Message-based communication** with unique Signal IDs (uint32_t)
- **Separate thread** for receiving messages per endpoint
- **Poll support** for integrating with event loops
- **Message filtering** by Signal ID or sender
- **Thread-safe message queues** with configurable size
- **Type-safe message payloads** using C++ templates
- **Connection verification** with ping/pong mechanism
- **Generic template framework** for extensible transport protocols
- **Interface-based identifier system** for type-safe addressing

## Quick Start

### Unix Domain Socket Example

```cpp
// Server
#include "AfUnixServer.h"
#include "LinxIpc.h"

auto server = AfUnixFactory::createServer("MyServer");
server->start();
auto msg = server->receive(INFINITE_TIMEOUT);

// Client
#include "AfUnixClient.h"
auto client = AfUnixFactory::createClient("MyClient");
client->connect(5000);
RawMessage message(20);
client->send(message);
```

### UDP Example

```cpp
// Server
#include "UdpLinx.h"

auto server = UdpFactory::createServer(8080);
server->start();
auto msg = server->receive(INFINITE_TIMEOUT);

// Client
auto client = UdpFactory::createClient("127.0.0.1", 8080);
client->connect(5000);
RawMessage message(20);
client->send(message);
```

## Architecture

### Key Components

- **IMessage/ILinxMessage**: Base message interface and template for typed messages
- **RawMessage**: Message class with dynamic byte buffer payload (alias for ILinxMessage<uint8_t>)
- **GenericServer/GenericClient**: Generic template framework for transport protocols
- **AfUnixServer/AfUnixClient**: Unix domain socket implementation
- **UdpServer/UdpClient**: UDP socket implementation (with multicast support)
- **AfUnixFactory/UdpFactory**: Protocol-specific factories for creating endpoints
- **LinxIpcHandler**: Message dispatcher with callback registration
- **IIdentifier**: Interface for client/server identification (StringIdentifier, PortInfo)

For detailed architecture information, see [ARCHITECTURE.md](ARCHITECTURE.md).

## Message API

### Creating Messages

Messages are identified by a `uint32_t` request ID.

**Empty message (RawMessage with no payload):**
```cpp
RawMessage msg(20);
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
RawMessage msg(20, buffer, sizeof(buffer));
```

**Message from vector:**
```cpp
std::vector<uint8_t> data = {1, 2, 3};
RawMessage msg(20, data);
```

**Typed message with custom payload:**
```cpp
struct Data {
    int value;
    char flag;
};
Data payload = {42, 'A'};
ILinxMessage<Data> msg(20, payload);
```

### Reading Message Payload

**Get typed payload from RawMessage:**
```cpp
RawMessage msg = ...;
Data *payload = msg.getPayloadAs<Data>();
printf("Value: %d\n", payload->value);
```

**Get typed payload from ILinxMessage:**
```cpp
ILinxMessage<Data> msg(20, {42, 'A'});
const Data *payload = msg.getPayload();
printf("Value: %d\n", payload->value);
```

**Get raw payload from RawMessage:**
```cpp
const uint8_t *raw = msg.getPayload();
```

**Get payload size:**
```cpp
uint32_t size = msg.getPayloadSize();
```

**Get request ID:**
```cpp
uint32_t reqId = msg.getReqId();
```

### Serializing Messages

Messages can be serialized to a byte buffer. The `serialize()` method returns the number of bytes written, or 0 on failure:

```cpp
RawMessage msg(20, buffer, size);
uint8_t serialBuffer[1024];
uint32_t bytesWritten = msg.serialize(serialBuffer, sizeof(serialBuffer));
if (bytesWritten == 0) {
    // Serialization failed (buffer too small)
} else {
    // Successfully serialized bytesWritten bytes
}
```

## Server API

### Creating a Server

**Unix Domain Socket Server:**
```cpp
#include "AfUnixServer.h"
#include "LinxIpc.h"

// Create server with default queue size (100)
auto server = AfUnixFactory::createServer("MyServer");

// Or specify custom queue size
auto server = AfUnixFactory::createServer("MyServer", 200);

// Start the server thread
server->start();
```

**UDP Server:**
```cpp
#include "UdpLinx.h"

// Create UDP server on port 8080
auto server = UdpFactory::createServer(8080);

// Create UDP server with multicast support
auto server = UdpFactory::createServer(8080, true);

// Create UDP server with custom queue size
auto server = UdpFactory::createServer(8080, false, 200);

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
           msg->from->format().c_str());
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
StringIdentifier clientId("ClientName");
auto msg = server->receive(INFINITE_TIMEOUT, LINX_ANY_SIG, &clientId);
```

**Send response to received message:**
```cpp
auto msg = server->receive(INFINITE_TIMEOUT);
if (msg) {
    LinxMessage response(100);
    msg->sendResponse(response);  // Send back to the sender
}
```

### Using Message Handlers

Register callbacks for specific message types:

```cpp
#include "LinxIpc.h"

auto server = AfUnixFactory::createServer("MyServer");
auto handler = LinxIpcHandler(server);

// Register callback for message ID 20
handler.registerCallback(20,
    [](const LinxReceivedMessageSharedPtr &msg, void *data) {
        printf("Received: 0x%x from %s\n",
               msg->message->getReqId(),
               msg->from->format().c_str());

        // Send response
        LinxMessage response(21);
        msg->sendResponse(response);
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

auto server = AfUnixFactory::createServer("MyServer");
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

**Unix Domain Socket Client:**
```cpp
#include "AfUnixClient.h"
#include "LinxIpc.h"

auto client = AfUnixFactory::createClient("MyClientName");
```

**UDP Client:**
```cpp
#include "UdpLinx.h"

// Connect to server at 192.168.1.100:8080
auto client = UdpFactory::createClient("192.168.1.100", 8080);

// For multicast
auto client = UdpFactory::createClient(LINX_MULTICAST_IP_ADDRESS, 8080);
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
    auto server = AfUnixFactory::createServer("MyServer");
    auto handler = LinxIpcHandler(server);

    handler.registerCallback(20,
        [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            printf("Received: 0x%x from %s\n",
                   msg->message->getReqId(),
                   msg->from->format().c_str());

            // Send response back
            LinxMessage response(21);
            msg->sendResponse(response);
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
    auto client = AfUnixFactory::createClient("MySender");

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

## UDP Communication Example

**UDP Server (udp_server.cpp):**
```cpp
#include <stdio.h>
#include "UdpLinx.h"
#include "LinxIpc.h"

int main() {
    // Create UDP server on port 8080
    auto server = UdpFactory::createServer(8080);
    auto handler = LinxIpcHandler(server);

    handler.registerCallback(20,
        [](const LinxReceivedMessageSharedPtr &msg, void *data) {
            printf("Received: 0x%x from %s\n",
                   msg->message->getReqId(),
                   msg->from->format().c_str());

            // Send response back
            LinxMessage response(21);
            msg->sendResponse(response);
            return 0;
        }, nullptr);

    handler.start();

    printf("UDP Server listening on port 8080\n");
    while (true) {
        handler.handleMessage(1000);
    }

    return 0;
}
```

**UDP Client (udp_client.cpp):**
```cpp
#include <stdio.h>
#include "UdpLinx.h"

int main() {
    // Connect to server at localhost:8080
    auto client = UdpFactory::createClient("127.0.0.1", 8080);

    if (!client->connect(5000)) {
        printf("Failed to connect\n");
        return -1;
    }

    LinxMessage msg(20);
    if (client->send(msg) < 0) {
        printf("Failed to send\n");
        return -1;
    }

    printf("UDP message sent successfully\n");
    return 0;
}
```

**UDP Multicast Example:**
```cpp
#include "UdpLinx.h"

// Server with multicast enabled
auto server = UdpFactory::createServer(8080, true);
server->start();

// Client connecting to multicast address
auto client = UdpFactory::createClient(LINX_MULTICAST_IP_ADDRESS, 8080);
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

### Basic Build

```bash
cmake -B build
cmake --build build
```

### Build with Logging

```bash
# Error level only
cmake -B build -DUSE_LOGGING=1
cmake --build build

# Warning and error
cmake -B build -DUSE_LOGGING=2
cmake --build build

# Info, warning, and error
cmake -B build -DUSE_LOGGING=3
cmake --build build

# All levels including debug
cmake -B build -DUSE_LOGGING=4
cmake --build build
```

### Output

Built libraries and test applications are placed in:
- Libraries: `build/output/lib/`
- Executables: `build/output/bin/`

## Testing

### Unit Tests

```bash
# Configure with unit tests enabled
cmake -B build_ut -DCMAKE_BUILD_TYPE=Debug -DUNIT_TESTS=ON

# Build all tests
cmake --build build_ut

# Run all tests
cd build_ut && ctest

# Run tests with verbose output
cd build_ut && ctest --verbose

# Run specific test
./build_ut/UnitTests/bin/LinxIpc-ut
```

### Test Applications

Example test applications are provided in the `TestApp/` directory:

```bash
# Build the project
cmake --build build

# Run test client
./build/output/bin/TestClient1

# Run IPC sender
./build/output/bin/ipcSender
```

## Project Structure

```
LinxIpc/                    # Main library
├── include/                # Public headers
│   ├── AfUnix.h           # Unix domain socket API
│   ├── UdpLinx.h          # UDP API
│   ├── LinxIpc.h          # Core IPC definitions
│   └── common/            # Common headers
├── src/                    # Implementation
│   ├── unix/              # Unix domain socket implementation
│   ├── udp/               # UDP implementation
│   ├── message/           # Message handling
│   └── queue/             # Queue management
└── tests/                  # Integration and performance tests

TestApp/                    # Example applications
trace/                      # Logging subsystem
UnitTests/                  # Unit test suite
```

## Thread Safety

- Server and Client objects are thread-safe for concurrent operations
- Message queues are protected with mutexes
- Each server runs its own receive thread

## Extending the Library

The LinxIpc framework is designed to be extensible. You can add support for new transport protocols by:

1. Implementing the socket interface for your transport protocol
2. Creating a new identifier type derived from `IIdentifier`
3. Using the `GenericServer` and `GenericClient` templates

For detailed information on extending the library, see [EXTENDING.md](EXTENDING.md).

## Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - Detailed architecture and design patterns
- [EXTENDING.md](EXTENDING.md) - Guide for adding new transport protocols
- [LICENSE.txt](LICENSE.txt) - License information

## License

See [LICENSE.txt](LICENSE.txt) for details.
