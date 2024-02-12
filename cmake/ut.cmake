function(add_to_ut)

    set(options OPTIONAL "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES INCLUDES MOCKS)
    cmake_parse_arguments(ADD_TO_UT "${options}" "${oneValueArgs}"
                        "${multiValueArgs}" ${ARGN} )

    set(GENERATED_MOCKS)
    foreach(file ${ADD_TO_UT_MOCKS})
        file(GLOB mocks "${file}/*Mock.cpp")
        list(APPEND GENERATED_MOCKS ${mocks})
    endforeach()

    target_sources(ut_mocks PRIVATE ${GENERATED_MOCKS})
    target_include_directories(ut_mocks
        PUBLIC
        ${ADD_TO_UT_INCLUDES}
        ${ADD_TO_UT_MOCKS}
    )

    get_target_property(TARGET_SOURCES ${ADD_TO_UT_TARGET} SOURCES)
    get_target_property(TARGET_INCLUDES ${ADD_TO_UT_TARGET} INCLUDE_DIRECTORIES)
    get_target_property(TARGET_DEFINITIONS ${ADD_TO_UT_TARGET} COMPILE_DEFINITIONS)

    if(TARGET_DEFINITIONS)
        target_compile_definitions(ut_sources PUBLIC ${TARGET_DEFINITIONS})
    endif()

    target_sources(ut_sources PRIVATE ${TARGET_SOURCES})
    target_include_directories(ut_sources
        PUBLIC
        ${ADD_TO_UT_INCLUDES}
        ${TARGET_INCLUDES}
    )

    target_sources(${PROJECT_NAME}-ut PRIVATE ${ADD_TO_UT_SOURCES})

endfunction()

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
    add_compile_options(-fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline)
    add_link_options(-lgcov -fprofile-arcs)
endif()

#############################
#     Generate Mocks
#############################

#############################
#     Compile Mocks library
#############################
add_library(ut_mocks OBJECT)

target_link_libraries(ut_mocks
    PUBLIC
    gmock
)

target_include_directories(ut_mocks
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cmock
)

#############################
#     Compile sources
#############################
add_library(ut_sources OBJECT)

#############################
#     Rerouted Objects
#############################
add_custom_target(ut_reroute
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmock/mock.py reroute --objects $<TARGET_OBJECTS:ut_sources> --mocks $<TARGET_OBJECTS:ut_mocks>
    DEPENDS ut_sources ut_mocks
    COMMAND_EXPAND_LISTS
)

#############################
#     Create Unit tests
#############################
add_executable(${PROJECT_NAME}-ut)

target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gtest gmock_main ut_mocks ut_sources
)

add_dependencies(${PROJECT_NAME}-ut ut_reroute)

add_custom_target(clear_gcda
    COMMAND find . -name *.gcda -exec rm {} "\\;"
    COMMAND rm -rf coverity_report
    COMMENT "Clear old gcda files"
)

if(VALGRIND)
    set(RUNNER valgrind --leak-check=full)
endif()

add_custom_target(run_${PROJECT_NAME}-ut
    COMMAND ${RUNNER} $<TARGET_FILE:${PROJECT_NAME}-ut> --gtest_output="xml:testResult.xml"
    DEPENDS $<$<BOOL:${COVERITY}>:clear_gcda> ${PROJECT_NAME}-ut
    COMMENT "Running unit tests"
)

if(COVERITY)
    add_custom_target(lcov_${PROJECT_NAME}-ut
        DEPENDS run_${PROJECT_NAME}-ut
        COMMAND lcov --capture --directory . --output-file coverage.info --gcov-tool=gcov-11 --rc lcov_branch_coverage=1
        COMMAND lcov -e coverage.info \"${PROJECT_SOURCE_DIR}/LinxIpc/src*\" -o coverage.info.filtered --rc lcov_branch_coverage=1
        #COMMAND lcov -r coverage.info.filtered \"${PROJECT_SOURCE_DIR}/LinxIpc/src/trace.cpp*\" -o coverage.info.filtered
        COMMAND genhtml coverage.info.filtered --branch-coverage --highlight --legend --output-directory coverity_report
        COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan
            "coverity report stored in ${CMAKE_BINARY_DIR}/coverity_report/index.html"
        COMMENT "Running coverity"
    )

    add_custom_target(gcovr_${PROJECT_NAME}-ut
        DEPENDS run_${PROJECT_NAME}-ut
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND mkdir -p ${CMAKE_BINARY_DIR}/coverity_report
        COMMAND gcovr -s -b --root ${PROJECT_SOURCE_DIR} --filter ${PROJECT_SOURCE_DIR}/LinxIpc/src --html-details ${CMAKE_BINARY_DIR}/coverity_report/coverity_report.html
        COMMENT "Running coverity"
    )
endif()
