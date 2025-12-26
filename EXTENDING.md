# Extending LinxIpc with New Socket Types

This guide demonstrates how to add a new socket type with a custom identifier to the LinxIpc framework.

## Architecture Overview

The framework uses a template-based design with these key components:

1. **Identifier Type**: Defines how to identify communication endpoints
2. **Socket Traits**: Maps socket types to their identifier types
3. **Socket Classes**: Handle low-level communication
4. **Generic Classes**: Provide common functionality via templates
   - `GenericClient<SocketType>`
   - `GenericServer<SocketType>`

**Key Design**: The identifier type is automatically extracted from the socket type using `SocketTraits`, eliminating redundant template parameters.

## Example: Adding a Bluetooth Socket

### Step 1: Define Your Identifier Type

Create `include/bluetooth/BluetoothTypes.h`:

```cpp
#pragma once

#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

// Bluetooth-specific identifier
struct BluetoothAddress {
    uint8_t address[6];  // MAC address
    uint16_t channel;

    bool operator==(const BluetoothAddress &other) const {
        return memcmp(address, other.address, 6) == 0 &&
               channel == other.channel;
    }

    // Format method for display/logging
    std::string format() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (int i = 0; i < 6; i++) {
            if (i > 0) oss << ":";
            oss << std::setw(2) << static_cast<int>(address[i]);
        }
        oss << " ch:" << std::dec << channel;
        return oss.str();
    }
};
```

**Important**: The `format()` method is optional. If your identifier type is `std::string` or can be implicitly converted to string, you don't need it.

### Step 2: Implement Socket Class

Create `src/bluetooth/BluetoothSocket.h`:

```cpp
#pragma once

#include "GenericSocket.h"
#include "BluetoothTypes.h"

class BluetoothSocket : public GenericSocket<BluetoothAddress> {
  public:
    BluetoothSocket();
    virtual ~BluetoothSocket();

    bool open() override;
    void close() override;
    int getFd() const override;
    int send(const LinxMessage &message, const BluetoothAddress &to) override;
    int receive(LinxMessagePtr *msg, BluetoothAddress *from, int timeout) override;
    int flush() override;

  protected:
    int fd;
    // ... Bluetooth-specific members
};
```

### Step 3: Create Client Class (Optional but Recommended)

Create `include/bluetooth/BluetoothClient.h`:

```cpp
#pragma once

#include "GenericClient.h"
#include "BluetoothTypes.h"

class BluetoothClientSocket;

class BluetoothClient : public GenericClient<BluetoothClientSocket> {
  public:
    BluetoothClient(const std::shared_ptr<BluetoothClientSocket> &socket,
                    const BluetoothAddress &address);
    ~BluetoothClient() = default;

    static std::shared_ptr<BluetoothClient> create(const uint8_t address[6],
                                                     uint16_t channel);
};
```

**Note**: The client class doesn't need to override anything - `getName()` automatically uses the identifier's `format()` method!

### Step 4: Register Socket Traits

Add socket traits specialization in your header:

```cpp
// In BluetoothTypes.h or your socket header
template<>
struct SocketTraits<BluetoothClientSocket> {
    using Identifier = BluetoothAddress;
};
```

### Step 5: Create Server

Create `include/bluetooth/BluetoothServer.h`:

```cpp
#pragma once

#include "GenericServer.h"
#include "BluetoothTypes.h"
#include <memory>

class LinxQueue;
class BluetoothServerSocket;

// Type alias for Bluetooth server
using BluetoothServer = GenericServer<BluetoothServerSocket>;

// Factory function
namespace BluetoothServerFactory {
    std::shared_ptr<BluetoothServer> create(const std::string &serviceName,
                                             uint16_t channel,
                                             size_t queueSize = LINX_DEFAULT_QUEUE_SIZE);
}
```

### Step 6: Implement the Classes
                                                         BluetoothAddress>> &server,
                      const BluetoothAddress &address);
    ~BluetoothEndpoint() = default;
};
```

**Note**: The endpoint also automatically uses the identifier's `format()` method - no overrides needed!

### Step 6: Implement the Classes

The implementations just need to call base class constructors and provide explicit template instantiations:

```cpp
// BluetoothClient.cpp
#include "BluetoothClient.h"
#include "BluetoothClientSocket.h"
#include "GenericClient.tpp"

template class GenericClient<BluetoothClientSocket>;

BluetoothClient::BluetoothClient(const std::shared_ptr<BluetoothClientSocket> &socket,
                                 const BluetoothAddress &address)
    : GenericClient<BluetoothClientSocket>(socket, address) {
}

std::shared_ptr<BluetoothClient> BluetoothClient::create(const uint8_t address[6],
                                                          uint16_t channel) {
    BluetoothAddress btAddr;
    memcpy(btAddr.address, address, 6);
    btAddr.channel = channel;

    auto socket = std::make_shared<BluetoothClientSocket>();
    return std::make_shared<BluetoothClient>(socket, btAddr);
}
```

```cpp
// BluetoothServer.cpp
#include "BluetoothServer.h"
#include "BluetoothServerSocket.h"
#include "LinxEventFd.h"
#include "LinxQueue.h"
#include "GenericServer.tpp"

namespace BluetoothServerFactory {

std::shared_ptr<BluetoothServer> create(const std::string &serviceName,
                                         uint16_t channel,
                                         size_t queueSize) {
    auto socket = std::make_shared<BluetoothServerSocket>(channel);
    auto efd = std::make_unique<LinxEventFd>();
    auto queue = std::make_unique<LinxQueue>(std::move(efd), queueSize);
    BluetoothAddress addr;
    // Initialize addr as needed
    return std::make_shared<BluetoothServer>(socket, std::move(queue), addr);
}

} // namespace BluetoothServerFactory

// Explicit template instantiation
template class GenericServer<BluetoothServerSocket>;
```
## Checklist for New Socket Type

- [ ] Create identifier type with `operator==` and optional `format()` method (e.g., `BluetoothTypes.h`)
- [ ] Implement socket classes inheriting from `GenericSocket<YourIdentifierType>`
- [ ] Register `SocketTraits` specialization mapping your socket type to identifier type
- [ ] Create client class inheriting from `GenericClient<YourSocket>`
- [ ] Create server type alias (e.g., `using YourServer = GenericServer<YourSocket>`)
- [ ] Add explicit template instantiations in .cpp files
- [ ] Write unit tests

That's it! The generic framework handles all the common functionality automatically, including name formatting via the identifier's `format()` method.
