# LinxIpc Distribution Guide

This document describes how to package and distribute the LinxIpc library to customers.

## Quick Start

### Option 1: Create Distribution Package (Recommended for Quick Distribution)
```bash
./create_package.sh 1.0.0
```

This creates a complete package with:
- Library files
- Headers
- Documentation
- Examples
- Ready-to-use archives (tar.gz and zip)

### Option 2: Install Locally (Recommended for System-Wide Installation)
```bash
# Configure with install prefix
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/LinxIpc

# Build
cmake --build build

# Install (may require sudo)
sudo cmake --install build
```

Result:
```
/opt/LinxIpc/
├── lib/
│   └── libLinxIpc.a
├── include/
│   └── LinxIpc/
│       ├── AfUnix.h
│       ├── UdpLinx.h
│       └── ...
└── lib/cmake/LinxIpc/
    ├── LinxIpcConfig.cmake
    ├── LinxIpcConfigVersion.cmake
    └── LinxIpcTargets.cmake
```

### Option 3: CMake Package (Professional Integration)

After installation, customers can use:

```cmake
# Customer's CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyApp)

# Find LinxIpc
find_package(LinxIpc 1.0 REQUIRED)

# Link to application
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE LinxIpc::LinxIpc)
```

**Complete working example:** See [examples/client-server-example/](examples/client-server-example/)

**Important:** If you build LinxIpc with `-DUSE_LOGGING=4`, this setting is automatically propagated to customer builds through CMake's target properties. Customers don't need to manually set USE_LOGGING.

## Distribution Methods

### 1. Distribution Package (create_package.sh)

**When to use:** Quick distribution, no system installation required

**Advantages:**
- Self-contained
- No installation needed
- Easy to verify contents
- Includes examples

**Usage:**
```bash
./create_package.sh 1.0.0
# Distribute: dist/LinxIpc-1.0.0.tar.gz
```

**Customer usage:**
```cmake
add_library(LinxIpc_imported STATIC IMPORTED)
set_target_properties(LinxIpc_imported PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/common;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/generic;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/message;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/trace"
    INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libtrace.a"
)
target_link_libraries(myapp PRIVATE LinxIpc_imported)
```

**Note:** LinxIpc's internal logging is already compiled in. If you built LinxIpc with `-DUSE_LOGGING=4`, you only need to define `USE_LOGGING` if you want to use trace macros in **your own code**. The `trace.h` header defaults to `USE_LOGGING=0` if not specified.

### 2. System Installation

**When to use:** System-wide deployment, multiple projects

**Advantages:**
- Standard CMake integration
- Version management
- Automatic dependency resolution

**Installation:**
```bash
# On build machine
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build

# Or create installation package
cmake -B build -DCMAKE_INSTALL_PREFIX=/tmp/LinxIpc-install
cmake --build build
cmake --install build
tar -czf LinxIpc-install.tar.gz -C /tmp LinxIpc-install
```

**Customer usage:**
```cmake
find_package(LinxIpc 1.0 REQUIRED)
target_link_libraries(myapp PRIVATE LinxIpc::LinxIpc)
```

### 3. CMake Package Configuration

**When to use:** Professional distribution, enterprise deployment

**Advantages:**
- Full CMake integration
- Version checking
- Component management
- Transitive dependencies

**Already configured in:** `LinxIpc/LinxIpcConfig.cmake.in`

## Building Packages

### Build Regular Package
```bash
# Clean build
cmake -B build
cmake --build build

# Create distribution
./create_package.sh 1.0.0
```

### Build Release Package
```bash
# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Create package
./create_package.sh 1.0.0
```

### Build with Coverage (Development)
```bash
# Development build with tests
cmake -B build_ut -DUNIT_TESTS=1 -DCOVERITY=1
cmake --build build_ut
cmake --build build_ut --target gcov_LinxIpc-ut
```

## Package Contents

The distribution package includes:

```
LinxIpc-1.0.0/
├── lib/
│   └── libLinxIpc.a           # Static library
├── include/
│   ├── AfUnix.h               # AF_UNIX interface
│   ├── UdpLinx.h              # UDP interface
│   ├── common/                # Common headers
│   ├── generic/               # Generic client/server
│   └── message/               # Message types
├── doc/
│   ├── README.md              # Project documentation
│   └── ARCHITECTURE.md        # Architecture guide
├── examples/
│   ├── simple_example.cpp     # Example code
│   └── CMakeLists.txt         # Example build
├── LICENSE.txt                # License
└── README.txt                 # Package usage guide
```

## Customer Integration Examples

### Example 1: Simple g++ Build
```bash
g++ -I./LinxIpc/include -I./LinxIpc/include/common -I./LinxIpc/include/generic \
    -I./LinxIpc/include/message -I./LinxIpc/include/trace \
    myapp.cpp LinxIpc/lib/libLinxIpc.a LinxIpc/lib/libtrace.a -o myapp -pthread
```

### Example 2: CMake Project
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp)

add_library(LinxIpc_imported STATIC IMPORTED)
set_target_properties(LinxIpc_imported PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/common;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/generic;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/message;${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/include/trace"
    INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/LinxIpc/lib/libtrace.a"
)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE LinxIpc_imported pthread)
```

### Example 3: Installed Package
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyApp)

find_package(LinxIpc 1.0 REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE LinxIpc::LinxIpc)
```

## Versioning

Update version in:
1. `LinxIpc/CMakeLists.txt` - write_basic_package_version_file()
2. `create_package.sh` - When calling the script

## Testing the Package

### Test Distribution Package
```bash
# Create and extract
./create_package.sh 1.0.0
cd dist
tar -xzf LinxIpc-1.0.0.tar.gz
cd LinxIpc-1.0.0/examples

# Build example
mkdir build && cd build
cmake ..
make
./simple_example
```

### Test Installation
```bash
# Install to temporary location
cmake -B build -DCMAKE_INSTALL_PREFIX=/tmp/test-install
cmake --build build
cmake --install build

# Test find_package
cd /tmp/test-project
cat > CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.20)
project(Test)
find_package(LinxIpc REQUIRED PATHS /tmp/test-install/lib/cmake)
message(STATUS "Found LinxIpc: \${LinxIpc_VERSION}")
EOF
cmake -B build
```

## Troubleshooting

### Package script fails
- Ensure build directory exists: `cmake -B build && cmake --build build`
- Check library location: `ls build/output/lib/libLinxIpc.a`

### CMake can't find package
- Set CMAKE_PREFIX_PATH: `cmake -DCMAKE_PREFIX_PATH=/path/to/install ..`
- Or use LinxIpc_DIR: `cmake -DLinxIpc_DIR=/path/to/install/lib/cmake/LinxIpc ..`

### Link errors
- Ensure all dependencies are linked: add `-pthread` for threading
- Check library path is correct
- Verify include directories are set

## Release Checklist

- [ ] Update version numbers
- [ ] Build release configuration
- [ ] Run all tests
- [ ] Generate coverage report
- [ ] Update documentation
- [ ] Create distribution package
- [ ] Test package on clean system
- [ ] Tag release in git
- [ ] Archive package
