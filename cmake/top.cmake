macro(determine_project_version)
    # Try to get exact tag on current commit
    execute_process(
        COMMAND git describe --exact-match --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE GIT_TAG_RESULT
    )

    if(GIT_TAG_RESULT EQUAL 0)
        # Current commit has a tag, use it
        set(PROJECT_VERSION_FULL ${GIT_TAG})
    else()
        # No tag on current commit, use latest tag + commit SHA
        execute_process(
            COMMAND git describe --tags --abbrev=0
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE LATEST_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE LATEST_TAG_RESULT
        )

        if(LATEST_TAG_RESULT EQUAL 0)
            # Get short commit SHA
            execute_process(
                COMMAND git rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE COMMIT_SHA
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            set(PROJECT_VERSION_FULL "${LATEST_TAG}-${COMMIT_SHA}")
        else()
            # Fallback if git is not available or no tags exist
            set(PROJECT_VERSION_FULL "1.0.0")
        endif()
    endif()

    # Extract numeric version (e.g., v2.0.2-5e9bf -> 2.0.2) for CMake VERSION parameter
    string(REGEX REPLACE "^v?([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" PROJECT_NUMERIC_VERSION ${PROJECT_VERSION_FULL})
endmacro()

function(add_to_ut)

    set(options OPTIONAL "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES MOCKS)
    cmake_parse_arguments(ADD_TO_UT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_TARGETS ${ADD_TO_UT_TARGET})
    set_target_properties(${ADD_TO_UT_TARGET}
        PROPERTIES
        UNIT_TEST_SOURCES "${ADD_TO_UT_SOURCES}"
        UNIT_TEST_MOCKS_DIR "${ADD_TO_UT_MOCKS}"
   )

endfunction()

macro(choice)

    set(options "")
    set(oneValueArgs VAR DESC)
    set(multivalueArgs VALUES)
    cmake_parse_arguments(MY_CHOICE "${options}" "${oneValueArgs}" "${multivalueArgs}" ${ARGN})

    if(NOT ${MY_CHOCE_VAR} IN_LIST MY_CHOICE_VALUES)
        message(FATAL_ERROR "Invalid value ${MY_CHOICE_VAR}: ${${MY_CHOICE_VAR}}. Please select one of ${MY_CHOICE_VALUES}")
    endif()

    set(${MY_CHOICE_VAR} ${${MY_CHOICE_VAR}} CACHE STRING ${MY_CHOICE_DESC})
    set_property(CACHE ${MY_CHOICE_VAR} PROPERTY STRINGS ${MY_CHOICE_VALUES})

    message(STATUS "${MY_CHOICE_VAR}=${${MY_CHOICE_VAR}}")
endmacro()

macro(add_option)

    set(options REQUIRED)
    set(oneValueArgs VAR DEFAULT)
    set(multivalueArgs "")
    cmake_parse_arguments(MY_CHOICE "${options}" "${oneValueArgs}" "${multivalueArgs}" ${ARGN})

    if(NOT DEFINED ${MY_CHOICE_VAR} AND DEFINED MY_CHOICE_DEFAULT)
        set(${MY_CHOICE_VAR} ${MY_CHOICE_DEFAULT})
    endif()

    if(NOT DEFINED ${MY_CHOICE_VAR})
        if(MY_CHOICE_REQUIRED)
            message(FATAL_ERROR "${MY_CHOICE_VAR} is required")
        endif()
    else()
        message(VERBOSE "option: ${MY_CHOICE_VAR}=${${MY_CHOICE_VAR}}")
        if(${MY_CHOICE_VAR} STREQUAL ON)
            add_compile_definitions(${MY_CHOICE_VAR})
        endif()

        if(NOT ${MY_CHOICE_VAR} STREQUAL ON AND NOT ${MY_CHOICE_VAR} STREQUAL OFF)
            add_compile_definitions(${MY_CHOICE_VAR}=${${MY_CHOICE_VAR}})
        endif()
    endif()
endmacro()

option(COVERITY "enable coverity" FALSE)
option(VALGRIND "enable valgrind" FALSE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/bin)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall -Wno-psabi -ffunction-sections -fdata-sections -fno-common -fstack-protector-strong -Werror
    "$<$<CONFIG:Debug>:SHELL:-Og>"
)

add_link_options(
    -Wl,-Map=output.map
    -Wl,--gc-sections)

if(COVERITY)
    add_compile_options(-fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline -g -O0)
    add_link_options(--coverage)
endif()

message(STATUS "#################################")
message(STATUS "     BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if(DEFINED TRACE_LEVEL)
    message(STATUS "     LOGGING_LEVEL: ${TRACE_LEVEL}")
endif()
if(UNIT_TESTS)
    message(STATUS "     TARGET: UNIT_TESTS")
    message(STATUS "         COVERITY: ${COVERITY}")
    message(STATUS "         VALGRIND: ${VALGRIND}")
else()
    message(STATUS "     TARGET: PROD")
endif()
message(STATUS "#################################")

add_option(VAR TRACE_LEVEL)
add_option(VAR UNIT_TESTS)
determine_project_version()