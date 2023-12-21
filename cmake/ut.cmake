include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY    https://github.com/google/googletest.git
    GIT_TAG           release-1.11.0
)

FetchContent_Populate(googletest)
message(STATUS "fetched googletest into ${googletest_SOURCE_DIR}")
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

add_compile_definitions(UNIT_TEST)

if(COVERITY)
    add_compile_options(-fprofile-arcs -ftest-coverage --coverage)
    add_link_options(-lgcov -fprofile-arcs -ftest-coverage --coverage)
endif()

get_property(UNIT_TEST_SRC GLOBAL PROPERTY UNIT_TEST_SRC)
get_property(UNIT_TEST_INCLUDE_DIRECTORIES GLOBAL PROPERTY UNIT_TEST_INCLUDE_DIRECTORIES)

add_executable(${PROJECT_NAME}-ut
    ${UNIT_TEST_SRC}
)

target_include_directories(${PROJECT_NAME}-ut
    PRIVATE
    ${UNIT_TEST_INCLUDE_DIRECTORIES}
)

target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gtest gmock gmock_main
)

add_custom_target(clear_gcda
    COMMAND find . -name *.gcda -exec rm {} "\\;"
    COMMENT "Clear old gcda files"
)

if(VALGRIND)
    set(RUNNER valgrind --leak-check=full)
endif()

add_custom_target(run_${PROJECT_NAME}-ut
    COMMAND ${RUNNER} $<TARGET_FILE:${PROJECT_NAME}-ut> --gtest_output="xml:testResult.xml"
    DEPENDS $<$<BOOL:COVERITY>:clear_gcda> ${PROJECT_NAME}-ut
    COMMENT "Running unit tests"
)

if(COVERITY)    
    get_filename_component(dep_path ${PROJECT_SOURCE_DIR}/LinxIpc ABSOLUTE)
    add_custom_target(coverity_${PROJECT_NAME}-ut
        DEPENDS clear_gcda
        DEPENDS run_${PROJECT_NAME}-ut
        COMMAND lcov --capture --directory . --output-file coverage.info --gcov-tool=gcov-8
        COMMAND lcov -e coverage.info \"${dep_path}/*\" -o coverage.info.filtered
        COMMAND lcov -r coverage.info \"${CMAKE_BINARY_DIR}/*\" -o coverage.info.filtered
        COMMAND genhtml coverage.info.filtered --output-directory coverity_report
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
            "coverity report stored in ${CMAKE_BINARY_DIR}/coverity_report/index.html"
        COMMENT "Running coverity"
    )
endif()