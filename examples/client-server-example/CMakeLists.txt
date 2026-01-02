cmake_minimum_required(VERSION 3.20)
project(LinxIpcClientApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the installed LinxIpc package
# CMake will look in standard locations and CMAKE_PREFIX_PATH
find_package(LinxIpc REQUIRED)

# Display what was found
message(STATUS "===========================================")
message(STATUS "LinxIpc Package Information:")
message(STATUS "  Version: ${LinxIpc_VERSION}")
message(STATUS "  Found: ${LinxIpc_FOUND}")
message(STATUS "===========================================")

# Create your application
add_executable(client_app client.cpp)

# Link against LinxIpc - this automatically handles:
# - Include directories
# - Library linking
# - Compiler flags
# - Transitive dependencies
target_link_libraries(client_app PRIVATE LinxIpc::LinxIpc)

# Create server application
add_executable(server_app server.cpp)
target_link_libraries(server_app PRIVATE LinxIpc::LinxIpc)

# That's it! No need to manually specify include paths or library paths
