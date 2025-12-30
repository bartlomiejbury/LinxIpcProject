include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           v1.16.0
)

FetchContent_Declare(
    cmock
    GIT_REPOSITORY    https://github.com/bartlomiejbury/cmock.git
    GIT_TAG           v2.0.1
)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(cmock)
message(STATUS "fetched googletest into ${googletest_SOURCE_DIR}")
message(STATUS "fetched cmock into ${cmock_SOURCE_DIR}")

include(${cmock_SOURCE_DIR}/mock.cmake)

#############################
#     get unittest targets
#############################
set(INCLUDE_TARGETS "" CACHE STRING "Include targets in UNIT_TEST_TARGETS")
set(EXCLUDE_TARGETS "" CACHE STRING "Exclude targets from UNIT_TEST_TARGETS")
get_property(UNIT_TEST_TARGETS GLOBAL PROPERTY UNIT_TEST_TARGETS)
if (EXCLUDE_TARGETS)
    list(FILTER UNIT_TEST_TARGETS EXCLUDE REGEX ${EXCLUDE_TARGETS})
endif()
if (INCLUDE_TARGETS)
    list(FILTER UNIT_TEST_TARGETS INCLUDE REGEX ${INCLUDE_TARGETS})
endif()
message(STATUS "Running unit tests for targets: ${UNIT_TEST_TARGETS}")

#############################
#     Generate mocks and reroute targets
#############################
foreach(TARGET ${UNIT_TEST_TARGETS})
    get_target_property(mocks ${TARGET} UNIT_TEST_MOCKS_DIR)
    list(APPEND UNIT_TEST_MOCKS_DIR ${mocks})
endforeach()

generate_cmock(
    TARGET ut_mocks
    PROXY_DIR proxy
    MOCKS_DIR ${UNIT_TEST_MOCKS_DIR}
)

foreach(TARGET ${UNIT_TEST_TARGETS})
    reroute_target(
        TARGET ${TARGET}
        CMOCK_TARGET ut_mocks
    )

    get_target_property(ut_sources ${TARGET} UNIT_TEST_SOURCES)
    list(APPEND UNIT_TEST_SOURCES ${ut_sources})

    get_target_property(libs ${TARGET} LINK_LIBRARIES)
    list(APPEND UNIT_TEST_LINKED_LIBS ${libs})

    list(APPEND UNIT_TEST_LINKED_LIBS ${TARGET}-rerouted)

endforeach()

list(REMOVE_ITEM UNIT_TEST_LINKED_LIBS ${UNIT_TEST_TARGETS} libs-NOTFOUND)
list(REMOVE_DUPLICATES UNIT_TEST_LINKED_LIBS)

#############################s
#     Compile Unit tests
#############################
add_executable(${PROJECT_NAME}-ut EXCLUDE_FROM_ALL ${UNIT_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gmock_main
    gtest
    ut_mocks
    ${UNIT_TEST_LINKED_LIBS}
)

#############################s
#     UT Runner
#############################

add_custom_target(clear_gcda
    COMMAND find . -name *.gcda -exec rm {} "\\;"
    COMMAND rm -rf coverity_report
    COMMENT "Clear old gcda files"
)

if(VALGRIND)
    set(RUNNER valgrind --leak-check=full)
endif()

add_custom_target(run_${PROJECT_NAME}-ut ALL
    COMMAND ${RUNNER} ${PROJECT_NAME}-ut --gtest_output="xml:testResult.xml"
    DEPENDS $<$<BOOL:${COVERITY}>:clear_gcda>
    COMMENT "Running unit tests"
    COMMAND_EXPAND_LISTS
)

#############################
#     Coverity
#############################
if(COVERITY)
    add_custom_target(gcov_${PROJECT_NAME}-ut
        DEPENDS run_${PROJECT_NAME}-ut
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/coverity_report
        COMMAND gcovr -s --txt-metric branch --root ${PROJECT_SOURCE_DIR} --filter ${PROJECT_SOURCE_DIR}/LinxIpc/src --html-details ${CMAKE_BINARY_DIR}/coverity_report/coverity_report.html --gcov-ignore-parse-errors=negative_hits.warn_once_per_file
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "coverity report stored in ${CMAKE_BINARY_DIR}/coverity_report/coverity_report.html"
        COMMENT "Running coverity"
    )
endif()

add_test(NAME "${PROJECT_NAME}_Unit_Tests" COMMAND ${CMAKE_BINARY_DIR}/output/bin/LinxIpc-ut)