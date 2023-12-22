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
    add_compile_options(-fprofile-arcs -ftest-coverage --coverage)
    add_link_options(-lgcov -fprofile-arcs -ftest-coverage --coverage)
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
#     Creating Rerouted Objects
#############################s
set(REROUTES_SYMBOL_FILE ${CMAKE_CURRENT_BINARY_DIR}/reroute.txt)

add_custom_target(ut_reroute
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmock/reroute.py $<TARGET_PROPERTY:ut_mocks,SOURCES> --output ${REROUTES_SYMBOL_FILE}
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmock/mock.sh -r ${REROUTES_SYMBOL_FILE} $<TARGET_OBJECTS:ut_sources>
    DEPENDS ut_sources
    COMMAND_EXPAND_LISTS
)

#############################
#     Creating Unit tests
#############################

add_executable(${PROJECT_NAME}-ut)

target_link_libraries(${PROJECT_NAME}-ut
    PRIVATE
    gtest gmock_main ut_mocks ut_sources
)

add_dependencies(${PROJECT_NAME}-ut ut_reroute)

add_custom_target(clear_gcda
    COMMAND find . -name *.gcda -exec rm {} "\\;"
    COMMENT "Clear old gcda files"
)

if(VALGRIND)
    set(RUNNER valgrind --leak-check=full)
endif()

add_custom_target(run_${PROJECT_NAME}-ut
    COMMAND ${RUNNER} $<TARGET_FILE:${PROJECT_NAME}-ut> --gtest_output="xml:testResult.xml"
    DEPENDS $<$<BOOL:${COVERITY}>:> ${PROJECT_NAME}-ut
    COMMENT "Running unit tests"
)

if(COVERITY)    
    get_filename_component(dep_path ${PROJECT_SOURCE_DIR}/LinxIpc ABSOLUTE)
    add_custom_target(coverity_${PROJECT_NAME}-ut
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
