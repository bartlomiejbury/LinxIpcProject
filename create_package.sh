#!/bin/bash
# LinxIpc Distribution Package Creator
# Creates a ready-to-use package with library and headers

set -e

VERSION=${1:-1.0.0}
PROJECT_ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$PROJECT_ROOT/build"
DIST_DIR="$PROJECT_ROOT/dist"
PACKAGE_NAME="LinxIpc-${VERSION}"
PACKAGE_DIR="$DIST_DIR/$PACKAGE_NAME"

echo "Creating LinxIpc distribution package v${VERSION}..."

# Check if library is built
if [ ! -f "$BUILD_DIR/output/lib/libLinxIpc.a" ]; then
    echo "Error: Library not found. Please build the project first:"
    echo "  cmake -B build"
    echo "  cmake --build build"
    exit 1
fi

# Clean and create directories
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"/{lib,include,doc,examples}

# Copy library
echo "Copying library..."
cp "$BUILD_DIR/output/lib/libLinxIpc.a" "$PACKAGE_DIR/lib/"

# Copy headers
echo "Copying headers..."
cp -r "$PROJECT_ROOT/LinxIpc/include"/* "$PACKAGE_DIR/include/"

# Copy trace headers
echo "Copying trace headers..."
mkdir -p "$PACKAGE_DIR/include/trace"
cp "$PROJECT_ROOT/trace/exports/trace.h" "$PACKAGE_DIR/include/trace/"

# Copy documentation files
echo "Copying documentation..."
if [ -f "$PROJECT_ROOT/README.md" ]; then
    cp "$PROJECT_ROOT/README.md" "$PACKAGE_DIR/doc/"
fi
if [ -f "$PROJECT_ROOT/ARCHITECTURE.md" ]; then
    cp "$PROJECT_ROOT/ARCHITECTURE.md" "$PACKAGE_DIR/doc/"
fi
if [ -f "$PROJECT_ROOT/LICENSE.txt" ]; then
    cp "$PROJECT_ROOT/LICENSE.txt" "$PACKAGE_DIR/"
fi

# Create README
cat > "$PACKAGE_DIR/README.txt" << 'EOF'
LinxIpc Library Package
=======================

Contents:
  lib/libLinxIpc.a     - Static library
  include/             - Header files
  doc/                 - Documentation
  examples/            - Usage examples

IMPORTANT - Logging Configuration:
-----------------------------------
If this library was built with USE_LOGGING (e.g., -DUSE_LOGGING=4),
you MUST define the same value when building your application:

  g++ -DUSE_LOGGING=4 -I./LinxIpc/include myapp.cpp LinxIpc/lib/libLinxIpc.a -o myapp

Or in CMakeLists.txt:
  target_compile_definitions(your_app PRIVATE USE_LOGGING=4)

Building with CMake:
--------------------
# Method 1: Using CMake Package (if installed system-wide)
find_package(LinxIpc REQUIRED)
target_link_libraries(your_app PRIVATE LinxIpc::LinxIpc)

# Method 2: Using IMPORTED target
add_library(LinxIpc STATIC IMPORTED)
set_target_properties(LinxIpc PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include"
)
target_link_libraries(your_app PRIVATE LinxIpc)

Building with g++:
------------------
g++ -I./LinxIpc/include myapp.cpp LinxIpc/lib/libLinxIpc.a -o myapp

# If library was built with USE_LOGGING=4:
g++ -DUSE_LOGGING=4 -I./LinxIpc/include myapp.cpp LinxIpc/lib/libLinxIpc.a -o myapp

Example Application:
--------------------
#include "AfUnix.h"
#include "UdpLinx.h"
#include <iostream>

int main() {
    // Create AF_UNIX server
    auto server = AfUnixFactory::createServer("MyServer", 10);

    // Create client
    auto client = AfUnixFactory::createClient("MyServer");

    // Send/receive messages
    // ... your code here

    return 0;
}

Requirements:
-------------
- C++17 or later
- Linux system with AF_UNIX and UDP socket support

EOF

# Create example
cat > "$PACKAGE_DIR/examples/simple_example.cpp" << 'EOF'
#include "AfUnix.h"
#include "LinxMessage.h"
#include <iostream>
#include <thread>

// Simple AF_UNIX communication example

void serverThread() {
    auto server = AfUnixFactory::createServer("ExampleServer", 10);
    if (!server) {
        std::cerr << "Failed to create server" << std::endl;
        return;
    }

    std::cout << "Server started, waiting for messages..." << std::endl;

    // Receive message
    auto msg = server->receive(5000, LINX_ANY_SIG);
    if (msg) {
        std::cout << "Server received message ID: " << msg->getReqId() << std::endl;
    }
}

void clientThread() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto client = AfUnixFactory::createClient("ExampleServer");
    if (!client) {
        std::cerr << "Failed to create client" << std::endl;
        return;
    }

    std::cout << "Client connected, sending message..." << std::endl;

    // Send message
    RawMessage msg(42);
    client->send(msg);

    std::cout << "Client sent message" << std::endl;
}

int main() {
    std::thread server(serverThread);
    std::thread client(clientThread);

    server.join();
    client.join();

    return 0;
}

// Build: g++ -I../include simple_example.cpp ../lib/libLinxIpc.a -o simple_example -pthread
EOF

# Create example CMakeLists.txt
cat > "$PACKAGE_DIR/examples/CMakeLists.txt" << 'EOF'
cmake_minimum_required(VERSION 3.20)
project(LinxIpcExample)

set(CMAKE_CXX_STANDARD 17)

# Add LinxIpc library
add_library(LinxIpc STATIC IMPORTED)
set_target_properties(LinxIpc PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../include;${CMAKE_CURRENT_SOURCE_DIR}/../include/common;${CMAKE_CURRENT_SOURCE_DIR}/../include/generic;${CMAKE_CURRENT_SOURCE_DIR}/../include/message"
)

# Build example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE LinxIpc pthread)
EOF

# Create packages
echo "Creating archives..."
cd "$DIST_DIR"

# Create tar.gz
tar -czf "${PACKAGE_NAME}.tar.gz" "$PACKAGE_NAME"
echo "Created: $DIST_DIR/${PACKAGE_NAME}.tar.gz"

# Create zip if available
if command -v zip &> /dev/null; then
    zip -r -q "${PACKAGE_NAME}.zip" "$PACKAGE_NAME"
    echo "Created: $DIST_DIR/${PACKAGE_NAME}.zip"
else
    echo "Note: zip not available, only tar.gz created"
    echo "Install with: sudo apt install zip"
fi

# Show package info
echo ""
echo "Package created successfully!"
echo "===================="
ls -lh "$DIST_DIR"/*.{tar.gz,zip} 2>/dev/null
echo ""
echo "Package contents:"
tar -tzf "${PACKAGE_NAME}.tar.gz" | head -20
echo "... (more files)"
echo ""
echo "To extract: tar -xzf ${PACKAGE_NAME}.tar.gz"
