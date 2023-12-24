option(UNIT_TESTS "build unit tests" FALSE)
option(COVERITY "enable coverity" FALSE)
option(VALGRIND "enable valgrind" FALSE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall -Wno-psabi -ffunction-sections -fdata-sections -fno-common -fstack-protector-strong -Werror
    $<$<CONFIG:Debug>:SHELL:-Og>
)

add_link_options(
    -Wl,-Map=output.map
    -Wl,--gc-sections
)

if(DEFINED UNIT_TESTS)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ut.cmake)
else()
    function(add_to_ut)
    endfunction()
endif()

message(STATUS "#################################")
message(STATUS "     BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if(DEFINED USE_LOGGING)
    message(STATUS "     LOGGING_LEVEL: ${USE_LOGGING}")
endif()
if(DEFINED UNIT_TESTS)
    message(STATUS "     TARGET: UNIT_TESTS")
    message(STATUS "         COVERITY: ${COVERITY}")
    message(STATUS "         VALGRIND: ${VALGRIND}")
else()
    message(STATUS "     TARGET: PROD")
endif()
message(STATUS "#################################")
