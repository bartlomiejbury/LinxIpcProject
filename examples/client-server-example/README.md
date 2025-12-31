# LinxIpc CMake Package Config Example

This example demonstrates how to use LinxIpc with CMake's `find_package()` mechanism.

## Examples Included

1. **client_app** - AfUnix client that sends messages to a server
2. **server_app** - AfUnix server that receives messages (like TestClient3.cpp)

## Prerequisites

LinxIpc must be installed on your system. Install it using:

```bash
cd /path/to/LinxIpcProject
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local  # or any install path
cmake --build build
sudo cmake --install build
```

## Building This Example

The example supports two build modes:

### Mode 1: Using Installed LinxIpc (Default)
LinxIpc must be installed first. Then build the example:

#### Method 1a: Standard Installation Location
If you installed to a standard location like `/usr/local`:

```bash
cmake -B build
cmake --build build
```

#### Method 1b: Custom Installation Location
If you installed to a custom location like `/opt/LinxIpc`:

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/opt/LinxIpc
cmake --build build
```

#### Method 1c: Using LinxIpc_DIR
Directly specify the CMake config location:

```bash
cmake -B build -DLinxIpc_DIR=/opt/LinxIpc/lib/cmake/LinxIpc
cmake --build build
```

### Mode 2: Building from Source (No Installation Required)
Build LinxIpc directly as part of this project:

```bash
cmake -B build -DUSE_INSTALLED_LINXIPC=OFF
cmake --build build
```

This mode:
- ✅ No installation required
- ✅ Builds LinxIpc from source automatically
- ✅ Useful for development and testing
- ⚠️  Rebuilds LinxIpc each time

## Running the Examples

### Server Application
Start the server in one terminal:
```bash
./build/server_app
```

Output:
```
===========================================
LinxIpc Server Application
===========================================

Creating AfUnix server 'ExampleServer'...
Server started, waiting for messages...
Press Ctrl+C to stop
```

### Client Application
In another terminal, send a message:
```bash
./build/client_app ExampleServer 42
```

Output:
```
===========================================
LinxIpc Client Application
===========================================

Creating AfUnix client...
Connecting to server: ExampleServer
Sending message: 42
Message sent successfully!
===========================================
```

Server will show:
```
Received message with reqId: 0x2a from: ExampleClient
```

### Info Mode
Run client without arguments to see library info:
```bash
./build/client_app
```

## How It Works

1. **find_package(LinxIpc)** locates the installed LinxIpcConfig.cmake file
2. The config file defines the `LinxIpc::LinxIpc` imported target
3. Linking against `LinxIpc::LinxIpc` automatically provides:
   - Include directories (no need for `target_include_directories`)
   - Library file (no need to specify .a path)
   - Compiler flags
   - All transitive dependencies

## Key Benefits

✅ **No manual paths** - CMake handles everything automatically
✅ **Version checking** - Can require specific versions
✅ **Relocatable** - Works regardless of install location
✅ **Standard** - Uses CMake best practices
✅ **Clean** - Minimal CMakeLists.txt

## Expected Output

```
===========================================
LinxIpc Package Information:
  Version: 1.0.0
  Found: TRUE
===========================================
...
===========================================
LinxIpc Client Application
===========================================

Available IPC mechanisms:
  - AF_UNIX (Unix Domain Sockets)
    Types: AfUnixClient, AfUnixServer
  - UDP (User Datagram Protocol)
    Types: UdpClient, UdpServer

Library successfully linked via CMake Package Config!
===========================================
```
