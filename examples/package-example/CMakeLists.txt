cmake_minimum_required(VERSION 3.20)
project(LinxIpcExample)

set(CMAKE_CXX_STANDARD 17)

# Add LinxIpc library
add_library(LinxIpc_imported STATIC IMPORTED)
set_target_properties(LinxIpc_imported PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libLinxIpc.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../include;${CMAKE_CURRENT_SOURCE_DIR}/../include/common;${CMAKE_CURRENT_SOURCE_DIR}/../include/generic;${CMAKE_CURRENT_SOURCE_DIR}/../include/message;${CMAKE_CURRENT_SOURCE_DIR}/../include/trace"
    INTERFACE_LINK_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libtrace.a"
)

# Build example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example PRIVATE LinxIpc_imported pthread)
