LinxIpc Library Package
=======================

Contents:
  lib/libLinxIpc.a     - Static library
  lib/libtrace.a       - Trace/logging library
  include/             - Header files
  doc/                 - Documentation
  examples/            - Usage examples

Logging Configuration (Optional):
----------------------------------
LinxIpc's internal logging is already compiled into the library and will work
automatically. You only need to define TRACE_LEVEL if you want to use trace
logging in YOUR OWN application code.

To enable trace macros in your application:
  g++ -DTRACE_LEVEL=4 -I./LinxIpc/include myapp.cpp LinxIpc/lib/libLinxIpc.a LinxIpc/lib/libtrace.a -o myapp

Or in CMakeLists.txt:
  target_compile_definitions(your_app PRIVATE TRACE_LEVEL=4)

Logging levels: 0=none, 1=error, 2=warning, 3=info, 4=debug

Building with CMake:
--------------------
# Using IMPORTED target (for package without system installation)
add_library(LinxIpc_imported STATIC IMPORTED)
set_target_properties(LinxIpc_imported PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/common;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/generic;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/message;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/trace"
    INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libtrace.a"
)
target_link_libraries(your_app PRIVATE LinxIpc_imported)

Building with g++:
------------------
g++ -I./LinxIpc/include -I./LinxIpc/include/common -I./LinxIpc/include/generic \
    -I./LinxIpc/include/message -I./LinxIpc/include/trace \
    myapp.cpp LinxIpc/lib/libLinxIpc.a LinxIpc/lib/libtrace.a -o myapp

# Optional: Enable trace logging in your own code:
g++ -DTRACE_LEVEL=4 -I./LinxIpc/include -I./LinxIpc/include/common \
    -I./LinxIpc/include/generic -I./LinxIpc/include/message -I./LinxIpc/include/trace \
    myapp.cpp LinxIpc/lib/libLinxIpc.a LinxIpc/lib/libtrace.a -o myapp

Example Application:
--------------------
#include "UnixLinx.h"
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
