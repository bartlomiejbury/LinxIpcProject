# LinxIpc Architecture

## Interface-Based Identifier System

The LinxIpc framework uses an interface-based design pattern for socket identifiers, providing type safety, extensibility, and polymorphism.

### Class Hierarchy

```
IIdentifier (interface)
├── StringIdentifier (Unix domain sockets)
└── PortInfo (UDP sockets)
```

### IIdentifier Interface

**File:** `LinxIpc/include/IIdentifier.h`

```cpp
class IIdentifier {
  public:
    virtual ~IIdentifier() = default;
    virtual std::string format() const = 0;
    virtual bool isEqual(const IIdentifier &other) const = 0;

    bool operator==(const IIdentifier &other) const {
        return isEqual(other);
    }
};
```

**Purpose:**
- Defines contract for all identifier types
- `format()`: Returns string representation for logging/display
- `isEqual()`: Type-safe equality comparison implementation
- `operator==`: Delegates to `isEqual()` for idiomatic C++ comparison

### StringIdentifier

**File:** `LinxIpc/include/AfUnix.h`

Wraps `std::string` for Unix domain socket paths.

```cpp
class StringIdentifier : public IIdentifier {
  public:
    StringIdentifier() = default;
    explicit StringIdentifier(const std::string &value);

    std::string format() const override;  // Returns the string
    bool isEqual(const IIdentifier &other) const override;
    const std::string& getValue() const;
};
```

**Usage:**
```cpp
StringIdentifier id("/tmp/my_socket");
std::string path = id.getValue();
```

### PortInfo

**File:** `LinxIpc/include/udp/UdpTypes.h`

Encapsulates IP address and port for UDP sockets.

```cpp
class PortInfo : public IIdentifier {
  public:
    PortInfo() = default;
    PortInfo(const std::string &ip, uint16_t port);

    std::string format() const override;  // Returns "ip:port"
    bool isEqual(const IIdentifier &other) const override;

    std::string ip;
    uint16_t port;
};
```

**Usage:**
```cpp
PortInfo info("192.168.1.100", 8080);
// format() returns "192.168.1.100:8080"
```

## Generic Template Framework

### GenericClient

**Template:** `GenericClient<SocketType>`

**IdentifierType** is automatically extracted from `SocketType` using `SocketTraits`.

Type aliases:
- `AfUnixClient = GenericClient<AfUnixSocket>`
- `UdpClient = GenericClient<UdpSocket>`

### GenericServer

**Template:** `GenericServer<SocketType>`

**IdentifierType** is automatically extracted from `SocketType` using `SocketTraits`.

Type aliases:
- `AfUnixServer = GenericServer<AfUnixSocket>`
- `UdpServer = GenericServer<UdpSocket>`

## Adding New Socket Types

To add a new socket type with custom identifier:

### 1. Create Identifier Class

```cpp
// MyCustomTypes.h
#include "IIdentifier.h"

class CustomIdentifier : public IIdentifier {
  public:
    CustomIdentifier() = default;
    CustomIdentifier(/* custom parameters */);

    std::string format() const override {
        // Return string representation
    }

    bool isEqual(const IIdentifier &other) const override {
        const auto *otherId = dynamic_cast<const CustomIdentifier*>(&other);
        return otherId && /* compare fields */;
    }

    // Custom fields and methods
};
```

### 2. Create Socket Implementation

```cpp
class MyCustomSocket : public GenericSocket<CustomIdentifier> {
  public:
    int send(const IMessage &msg, const CustomIdentifier &to) override;
    int receive(RawMessagePtr *msg, std::unique_ptr<IIdentifier> *from, int timeoutMs) override;
    // ... other methods
};
```

### 3. Register Socket Traits

Add a `SocketTraits` specialization in your header file:

```cpp
// Specialize SocketTraits for your socket type
template<>
struct SocketTraits<MyCustomSocket> {
    using Identifier = CustomIdentifier;
};
```

### 4. Define Type Aliases

```cpp
// MyCustomClient.h
#include "GenericClient.h"
using MyCustomClient = GenericClient<MyCustomSocket>;

// MyCustomServer.h
#include "GenericServer.h"
using MyCustomServer = GenericServer<MyCustomSocket>;
```

### 5. Create Factory Functions

```cpp
namespace MyCustomClientFactory {
    std::shared_ptr<MyCustomClient> create(/* parameters */);
}

namespace MyCustomServerFactory {
    std::shared_ptr<MyCustomServer> create(/* parameters */);
}
```

### 6. Template Instantiation

```cpp
// In .cpp files
template class GenericClient<MyCustomSocket>;
template class GenericServer<MyCustomSocket>;
```

## Benefits

1. **Type Safety**: Compile-time checking of identifier types
2. **Polymorphism**: Interface allows runtime type checking via `isEqual()` and `operator==`
3. **Extensibility**: New socket types don't require modifying generic templates
4. **Zero Overhead**: Template instantiation eliminates abstraction cost
5. **Testability**: Each identifier type can be tested independently
6. **Documentation**: `format()` method provides consistent string representation
7. **Move Semantics**: Uses `std::unique_ptr` for efficient ownership transfer without cloning

## Design Patterns

- **Template Method**: GenericClient/Server define algorithm skeleton
- **Strategy**: GenericSocket defines socket behavior interface
- **Factory**: Separate factory namespaces for object creation
- **Interface Segregation**: IIdentifier defines minimal contract
- **Type Aliases**: Simplify complex template types
