cmake_minimum_required(VERSION 3.14)
project(LinxIpc VERSION 1.0.0 LANGUAGES CXX)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/top.cmake)

set(LINX_LOG_PREFIX trace)

add_subdirectory(trace)
add_subdirectory(LinxIpc)
add_subdirectory(TestApp)

if(UNIT_TESTS)
    add_subdirectory(UnitTests)
endif()
