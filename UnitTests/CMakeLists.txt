include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           v1.16.0
)

FetchContent_Declare(
    cmock
    GIT_REPOSITORY    https://github.com/bartlomiejbury/cmock.git
    GIT_TAG           v2.0.0
)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(cmock)
message(STATUS "fetched googletest into ${googletest_SOURCE_DIR}")
message(STATUS "fetched cmock into ${cmock_SOURCE_DIR}")

include(${cmock_SOURCE_DIR}/mock.cmake)

get_property(UNIT_TEST_TARGETS GLOBAL PROPERTY UNIT_TEST_TARGETS)
get_property(UNIT_TEST_MOCKS_DIR GLOBAL PROPERTY UNIT_TEST_MOCKS_DIR)
get_property(UNIT_TEST_SOURCES GLOBAL PROPERTY UNIT_TEST_SOURCES)

if(COVERITY)
    add_compile_options(-fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline -O0)
    add_link_options(--coverage)
endif()

#############################
#     Compile Unit tests
#############################
add_executable(${PROJECT_NAME}-ut EXCLUDE_FROM_ALL ${UNIT_TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gmock_main
    gtest
    ut_mocks
)

#############################
#     Generate Mocks
#############################
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
    target_link_libraries(${PROJECT_NAME}-ut
        PRIVATE
        ${TARGET}-rerouted
    )
endforeach()

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
    add_custom_target(lcov_${PROJECT_NAME}-ut
        DEPENDS run_${PROJECT_NAME}-ut
        COMMAND lcov --capture --directory . --output-file coverage.info --gcov-tool=gcov-11 --rc lcov_branch_coverage=1
        COMMAND lcov -e coverage.info \"${PROJECT_SOURCE_DIR}/LinxIpc/src*\" -o coverage.info.filtered --rc lcov_branch_coverage=1
        #COMMAND lcov -r coverage.info.filtered \"${PROJECT_SOURCE_DIR}/LinxIpc/src/trace.cpp*\" -o coverage.info.filtered
        COMMAND genhtml coverage.info.filtered --branch-coverage --no-function-coverage --highlight --legend --output-directory coverity_report
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "coverity report stored in ${CMAKE_BINARY_DIR}/coverity_report/index.html"
        COMMENT "Running coverity"
    )

    add_custom_target(gcovr_${PROJECT_NAME}-ut
        DEPENDS run_${PROJECT_NAME}-ut
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/coverity_report
        COMMAND gcovr -s -b --root ${PROJECT_SOURCE_DIR} --filter ${PROJECT_SOURCE_DIR}/LinxIpc/src --html-details ${CMAKE_BINARY_DIR}/coverity_report/coverity_report.html
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "coverity report stored in ${CMAKE_BINARY_DIR}/coverity_report/index.html"
        COMMENT "Running coverity"
    )
endif()

enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} --build . --target run_LinxIpc-ut)
