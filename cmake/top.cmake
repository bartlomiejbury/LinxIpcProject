option(UNIT_TESTS "build unit tests" FALSE)
option(COVERITY "enable coverity" FALSE)
option(VALGRIND "enable valgrind" FALSE)

define_property(GLOBAL PROPERTY UNIT_TEST_MOCKS BRIEF_DOCS "aa " FULL_DOCS "mock headers")
define_property(GLOBAL PROPERTY UNIT_TEST_SOURCES BRIEF_DOCS "aa " FULL_DOCS "ut sources")
define_property(GLOBAL PROPERTY UNIT_TEST_MOCKS_DIR BRIEF_DOCS "aa " FULL_DOCS "ut sources")

define_property(GLOBAL PROPERTY UNIT_TEST_TARGET_DEFINES BRIEF_DOCS "aa " FULL_DOCS "target defines")
define_property(GLOBAL PROPERTY UNIT_TEST_TARGET_SOURCES BRIEF_DOCS "aa " FULL_DOCS "target sources")
define_property(GLOBAL PROPERTY UNIT_TEST_TARGET_INCLUDES BRIEF_DOCS "aa " FULL_DOCS "target includes")

function(add_to_ut)

    set(options OPTIONAL "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES MOCKS)
    cmake_parse_arguments(ADD_TO_UT "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    get_property(ut_sources GLOBAL PROPERTY UNIT_TEST_SOURCES)
    list(APPEND ut_sources ${ADD_TO_UT_SOURCES})
    set_property(GLOBAL PROPERTY UNIT_TEST_SOURCES ${ut_sources})

    get_property(ut_mocks_dir GLOBAL PROPERTY UNIT_TEST_MOCKS_DIR)
    list(APPEND ut_mocks_dir ${ADD_TO_UT_MOCKS})
    set_property(GLOBAL PROPERTY UNIT_TEST_MOCKS_DIR ${ut_mocks_dir})

    foreach(file ${ADD_TO_UT_MOCKS})
        file(GLOB mocks "${file}/*Mock.h")

        get_property(current_mocks GLOBAL PROPERTY UNIT_TEST_MOCKS)
        list(APPEND current_mocks ${mocks})
        set_property(GLOBAL PROPERTY UNIT_TEST_MOCKS ${current_mocks})

    endforeach()
                
    get_target_property(TARGET_SOURCES ${ADD_TO_UT_TARGET} SOURCES)
    get_target_property(TARGET_INCLUDES ${ADD_TO_UT_TARGET} INCLUDE_DIRECTORIES)
    get_target_property(TARGET_DEFINITIONS ${ADD_TO_UT_TARGET} COMPILE_DEFINITIONS)

    if(TARGET_DEFINITIONS)
        get_property(current_defines GLOBAL PROPERTY UNIT_TEST_TARGET_DEFINES)
        list(APPEND current_defines ${TARGET_DEFINITIONS})
        set_property(GLOBAL PROPERTY UNIT_TEST_TARGET_DEFINES ${current_defines})
    endif()

    get_property(current_sources GLOBAL PROPERTY UNIT_TEST_TARGET_SOURCES)
    list(APPEND current_sources ${TARGET_SOURCES})
    set_property(GLOBAL PROPERTY UNIT_TEST_TARGET_SOURCES ${current_sources})

    get_property(current_includes GLOBAL PROPERTY UNIT_TEST_TARGET_INCLUDES)
    list(APPEND current_includes ${TARGET_INCLUDES})
    set_property(GLOBAL PROPERTY UNIT_TEST_TARGET_INCLUDES ${current_includes})

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
