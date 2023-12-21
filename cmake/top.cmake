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

macro(add_to_ut)
    set(options OPTIONAL "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(ADD_TO_UT "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_SRC 
        $<TARGET_PROPERTY:${ADD_TO_UT_TARGET},SOURCES>
        ${ADD_TO_UT_SOURCES}
    )
    set_property(GLOBAL APPEND PROPERTY UNIT_TEST_INCLUDE_DIRECTORIES
        $<TARGET_PROPERTY:${ADD_TO_UT_TARGET},INCLUDE_DIRECTORIES>
    )
endmacro()
