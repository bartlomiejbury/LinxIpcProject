cmake_minimum_required(VERSION 3.14)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/top.cmake)

project(LinxIpc VERSION ${PROJECT_NUMERIC_VERSION} LANGUAGES CXX)
message(STATUS "LinxIpc Version: ${PROJECT_VERSION_FULL}")
message(STATUS "LinxIpc Numeric Version: ${PROJECT_NUMERIC_VERSION}")

option(LINXIPC_INSTALL "Install LinxIpc library" ON)

add_subdirectory(trace)
add_subdirectory(LinxIpc)
add_subdirectory(examples EXCLUDE_FROM_ALL)

if(UNIT_TESTS)
    enable_testing()
    add_subdirectory(UnitTests)
endif()
