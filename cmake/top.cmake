option(UNIT_TESTS "build unit tests" FALSE)
option(COVERITY "enable coverity" FALSE)
option(VALGRIND "enable valgrind" FALSE)

function(add_to_ut)

    set(options OPTIONAL "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES MOCKS)
    cmake_parse_arguments(ADD_TO_UT "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_SOURCES ${ADD_TO_UT_SOURCES})
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_MOCKS_DIR ${ADD_TO_UT_MOCKS})
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_TARGET_SOURCES $<TARGET_PROPERTY:${ADD_TO_UT_TARGET},SOURCES>)
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_TARGET_INCLUDES $<TARGET_PROPERTY:${ADD_TO_UT_TARGET},INCLUDE_DIRECTORIES>)
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_TARGET_DEFINES $<TARGET_PROPERTY:${ADD_TO_UT_TARGET},COMPILE_DEFINITIONS>)

endfunction()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall -Wno-psabi -ffunction-sections -fdata-sections -fno-common -fstack-protector-strong -Werror
    $<$<CONFIG:Debug>:SHELL:-O0>
)

add_link_options(
    -Wl,-Map=output.map
    -Wl,--gc-sections
)

message(STATUS "#################################")
message(STATUS "     BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if(DEFINED USE_LOGGING)
    message(STATUS "     LOGGING_LEVEL: ${USE_LOGGING}")
endif()
if(UNIT_TESTS)
    message(STATUS "     TARGET: UNIT_TESTS")
    message(STATUS "         COVERITY: ${COVERITY}")
    message(STATUS "         VALGRIND: ${VALGRIND}")
else()
    message(STATUS "     TARGET: PROD")
endif()
message(STATUS "#################################")
