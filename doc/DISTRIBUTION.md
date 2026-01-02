# LinxIpc Distribution Guide

This document describes how to package and distribute the LinxIpc library to customers.

## Quick Start

### Option 1: Create Distribution Package (Recommended for Quick Distribution)
```bash
# Automatically detects version from git tags, uses default directories
./create_package.sh
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
cmake -B build

# Build
cmake --build build

# Install (may require sudo)
sudo cmake --install build --prefix /opt/LinxIpc
```

Result:
```
/opt/LinxIpc/
├── lib/
│   ├── libLinxIpc.a
|   ├── libtrace.a
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

# Find LinxIpc (accepts any version)
find_package(LinxIpc REQUIRED)

# Link to application
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE LinxIpc::LinxIpc)
```

**Complete working example:** See [examples/client-server-example/](examples/client-server-example/)

**Important:** If you build LinxIpc with `-DTRACE_LEVEL=4`, this setting is automatically propagated to customer builds through CMake's target properties. Customers don't need to manually set TRACE_LEVEL.

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
# Use git-detected version (tar.gz created in current directory)
./create_package.sh
# Creates: LinxIpc-v2.0.2-5e9bf.tar.gz (if untagged commit)
# or LinxIpc-v2.0.2.tar.gz (if tagged commit)

# Specify version manually
./create_package.sh -v 1.0.0
# Creates: LinxIpc-1.0.0.tar.gz

# Use custom build directory (e.g., for release builds)
./create_package.sh -b build_release

# Use custom staging directory
./create_package.sh -d /tmp/dist

# Full control
./create_package.sh -v 2.0.0 -b build_release -d /tmp/dist
```

**Notes:**
- The tar.gz archive is created in the **current working directory** (where you invoke the script)
- `-d DIST_DIR` specifies the staging directory (default: `./dist`) for unpacking and preparing files
- `-b BUILD_DIR` specifies the build directory to package from (default: `./build`)
- `-v VERSION` overrides automatic git version detection

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

**Note:** LinxIpc's internal logging is already compiled in. If you built LinxIpc with `-DTRACE_LEVEL=4`, you only need to define `TRACE_LEVEL` if you want to use trace macros in **your own code**. The `trace.h` header defaults to `TRACE_LEVEL=0` if not specified.

### 2. System Installation

**When to use:** System-wide deployment, multiple projects

**Advantages:**
- Standard CMake integration
- Version management
- Automatic dependency resolution

**Installation:**
```bash
# On build machine
cmake -B build
cmake --build build
sudo cmake --install build

# Or create installation package
cmake -B build
cmake --build build
cmake --install build --prefix /tmp/LinxIpc-install
tar -czf LinxIpc-install.tar.gz -C /tmp LinxIpc-install
```

**Customer usage:**
```cmake
find_package(LinxIpc REQUIRED)
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
# Clean build (version auto-detected from git)
cmake -B build
cmake --build build

# Create distribution (version auto-detected from git)
./create_package.sh
```

### Build Release Package
```bash
# Release build (version auto-detected from git)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Create package (version auto-detected from git)
./create_package.sh
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
LinxIpc-<version>/  # e.g., LinxIpc-v2.0.2/ or LinxIpc-v2.0.2-5e9bf/
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

find_package(LinxIpc REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE LinxIpc::LinxIpc)
```

## Versioning

LinxIpc uses **automatic git-based versioning**:

### How It Works
- **Tagged commit**: Uses the git tag as-is (e.g., `v2.0.2`)
- **Untagged commit**: Uses latest tag + short commit SHA (e.g., `v2.0.2-5e9bf`)
- **No tags**: Falls back to `1.0.0`

### Version Detection
Both CMake and `create_package.sh` automatically detect the version:

```bash
# CMake build - version detected automatically
cmake -B build
cmake --build build

# Package creation - version detected automatically
./create_package.sh

# Or override with specific version
./create_package.sh 2.1.0
```

### Creating New Versions
```bash
# Create a new version tag
git tag -a v2.0.2 -m "Release version 2.0.2"

# Push the tag
git push origin v2.0.2

# Build and package will now use v2.0.2
cmake -B build
./create_package.sh
```

### Version Variables
- `PROJECT_VERSION_FULL`: Complete version with commit hash (e.g., `v2.0.2-5e9bf`)
- `PROJECT_VERSION`: Numeric version only (e.g., `2.0.2`) - used for CMake compatibility
- `LinxIpc_VERSION`: Version exposed to customers via `find_package()`

## Testing the Package

### Test Distribution Package
```bash
# Create and extract (version auto-detected)
./create_package.sh
cd dist
tar -xzf LinxIpc-*.tar.gz  # Extract the created package
cd LinxIpc-*/examples      # Enter the versioned directory

# Build example
mkdir build && cd build
cmake ..
make
./simple_example
```

### Test Installation
```bash
# Install to temporary location
cmake -B build
cmake --build build
cmake --install build --prefix =/tmp/test-install

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

- [ ] Create git tag for new version (e.g., `git tag -a v2.0.2 -m "Release v2.0.2"`)
- [ ] Push tag to repository (`git push origin v2.0.2`)
- [ ] Build release configuration (`cmake -B build -DCMAKE_BUILD_TYPE=Release`)
- [ ] Run all tests
- [ ] Generate coverage report
- [ ] Update documentation
- [ ] Create distribution package (`./create_package.sh` - uses git tag automatically)
- [ ] Test package on clean system
- [ ] Archive package
