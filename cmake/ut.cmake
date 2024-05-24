include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           release-1.11.0
)

FetchContent_Populate(googletest)
message(STATUS "fetched googletest into ${googletest_SOURCE_DIR}")
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

get_property(UNIT_TEST_TARGET_DEFINES GLOBAL PROPERTY UNIT_TEST_TARGET_DEFINES)
get_property(UNIT_TEST_TARGET_SOURCES GLOBAL PROPERTY UNIT_TEST_TARGET_SOURCES)
get_property(UNIT_TEST_TARGET_INCLUDES GLOBAL PROPERTY UNIT_TEST_TARGET_INCLUDES)
get_property(UNIT_TEST_SOURCES GLOBAL PROPERTY UNIT_TEST_SOURCES)
get_property(UNIT_TEST_MOCKS_DIR GLOBAL PROPERTY UNIT_TEST_MOCKS_DIR)

foreach(file ${UNIT_TEST_MOCKS_DIR})
    file(GLOB mocks "${file}/*Mock.h")
    list(APPEND UNIT_TEST_MOCKS ${mocks})
endforeach()

add_compile_definitions(UNIT_TEST)

if(COVERITY)
    add_compile_options(-fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline -O0)
    add_link_options(--coverage)
endif()

#############################
#     Generate Mocks
#############################

set(GENERATED_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/proxy)
file(REMOVE_RECURSE ${GENERATED_SOURCE_DIR})
file(MAKE_DIRECTORY ${GENERATED_SOURCE_DIR})

execute_process(
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmock/mock.py checkHeaders --headers ${UNIT_TEST_MOCKS} --output ${GENERATED_SOURCE_DIR}
    OUTPUT_VARIABLE GENERATED_MOCKS
)

if(NOT GENERATED_MOCKS)
    message(STATUS "No mocks were generate create dumy file")
    file(WRITE ${GENERATED_SOURCE_DIR}/dummy.cpp "")
    set(GENERATED_MOCKS ${GENERATED_SOURCE_DIR}/dummy.cpp)
endif()

add_custom_command(
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmock/mock.py generate --headers ${UNIT_TEST_MOCKS} --output ${GENERATED_SOURCE_DIR}
    OUTPUT ${GENERATED_MOCKS}
    DEPENDS ${UNIT_TEST_MOCKS}
    COMMENT "Generate mocks"
)

add_library(ut_mocks OBJECT ${GENERATED_MOCKS})

target_link_libraries(ut_mocks
    PUBLIC
    gmock
)

target_include_directories(ut_mocks
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cmock
    ${UNIT_TEST_MOCKS_DIR}
    ${UNIT_TEST_TARGET_INCLUDES}
)

#############################
#     Compile sources
#############################
add_library(ut_sources OBJECT ${UNIT_TEST_TARGET_SOURCES})
target_include_directories(ut_sources PUBLIC ${UNIT_TEST_TARGET_INCLUDES})
target_compile_definitions(ut_sources PUBLIC ${UNIT_TEST_TARGET_DEFINES})

#############################
#     Rerouted Objects
#############################
add_custom_target(ut_reroute
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmock/mock.py reroute --objects $<TARGET_OBJECTS:ut_sources> --mocks $<TARGET_OBJECTS:ut_mocks>
    DEPENDS ut_sources ut_mocks
    COMMAND_EXPAND_LISTS
    COMMENT "Reroute objects"
)

#############################
#     Compile Unit tests
#############################
add_executable(${PROJECT_NAME}-ut ${UNIT_TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gtest gmock_main ut_mocks ut_sources
)


add_dependencies(${PROJECT_NAME}-ut ut_reroute)

#############################
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

add_custom_target(run_${PROJECT_NAME}-ut
    COMMAND ${RUNNER} ${PROJECT_NAME}-ut --gtest_output="xml:testResult.xml"
    DEPENDS $<$<BOOL:${COVERITY}>:clear_gcda>
    COMMENT "Running unit tests"
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
