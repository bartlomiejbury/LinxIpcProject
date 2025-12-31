cmake_minimum_required(VERSION 3.20)
project(LinxIpcClientApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build with installed LinxIpc or from source
option(USE_INSTALLED_LINXIPC "Use installed LinxIpc via find_package()" ON)

if(USE_INSTALLED_LINXIPC)
    # Find the installed LinxIpc package
    # CMake will look in standard locations and CMAKE_PREFIX_PATH
    find_package(LinxIpc 1.0 REQUIRED)

    # Display what was found
    message(STATUS "===========================================")
    message(STATUS "LinxIpc Package Information:")
    message(STATUS "  Version: ${LinxIpc_VERSION}")
    message(STATUS "  Found: ${LinxIpc_FOUND}")
    message(STATUS "===========================================")
else()
    # Build LinxIpc from source
    message(STATUS "===========================================")
    message(STATUS "Building LinxIpc from source")
    message(STATUS "===========================================")

    # Include cmake utilities from parent project
    set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
    include(${PROJECT_SOURCE_DIR}/cmake/top.cmake)

    # Add the LinxIpc directory from parent project
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../LinxIpc
                     ${CMAKE_BINARY_DIR}/LinxIpc)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../trace
                     ${CMAKE_BINARY_DIR}/trace)
endif()

# Create your application
add_executable(client_app client.cpp)

# Link against LinxIpc - this automatically handles:
# - Include directories
# - Library linking
# - Compiler flags
# - Transitive dependencies
if(USE_INSTALLED_LINXIPC)
    target_link_libraries(client_app PRIVATE LinxIpc::LinxIpc)
else()
    target_link_libraries(client_app PRIVATE LinxIpc trace)
endif()

# Create server application
add_executable(server_app server.cpp)
if(USE_INSTALLED_LINXIPC)
    target_link_libraries(server_app PRIVATE LinxIpc::LinxIpc)
else()
    target_link_libraries(server_app PRIVATE LinxIpc trace)
endif()

# That's it! No need to manually specify include paths or library paths
