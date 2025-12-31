
add_option(VAR LINX_LOG_PREFIX)

add_library(LinxIpc STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/message/RawMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/common/LinxIpcHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixSocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/udp/UdpFactory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/udp/UdpSocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxQueue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxEventFd.cpp
    $<TARGET_OBJECTS:trace>
)

target_include_directories(
    LinxIpc
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/common>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/generic>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/message>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/trace/exports>
    $<INSTALL_INTERFACE:include/LinxIpc>
    $<INSTALL_INTERFACE:include/LinxIpc/common>
    $<INSTALL_INTERFACE:include/LinxIpc/generic>
    $<INSTALL_INTERFACE:include/LinxIpc/message>
    $<INSTALL_INTERFACE:include/LinxIpc/trace>
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/message
    ${CMAKE_CURRENT_LIST_DIR}/src/unix
    ${CMAKE_CURRENT_LIST_DIR}/src/udp
    ${CMAKE_CURRENT_LIST_DIR}/src/common
    ${CMAKE_CURRENT_LIST_DIR}/src/queue
)

target_link_libraries(LinxIpc
    PRIVATE
)

# Propagate USE_LOGGING definition to consumers
# This is needed because trace.h uses USE_LOGGING in macros
if(DEFINED USE_LOGGING)
    target_compile_definitions(LinxIpc PUBLIC USE_LOGGING=${USE_LOGGING})
else()
    # Default to no logging if not specified
    target_compile_definitions(LinxIpc PUBLIC USE_LOGGING=0)
endif()

# Installation rules
install(TARGETS LinxIpc
    EXPORT LinxIpcTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install public headers
install(DIRECTORY include/
    DESTINATION include/LinxIpc
    FILES_MATCHING PATTERN "*.h"
)

# Install package configuration files
include(CMakePackageConfigHelpers)

# Generate the config file that includes the exports
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/LinxIpcConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LinxIpcConfig.cmake
    INSTALL_DESTINATION lib/cmake/LinxIpc
)

# Generate the version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/LinxIpcConfigVersion.cmake
    VERSION 1.0.0
    COMPATIBILITY SameMajorVersion
)

# Install the config and version files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LinxIpcConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LinxIpcConfigVersion.cmake
    DESTINATION lib/cmake/LinxIpc
)

# Install the export set for use with the install-tree
install(EXPORT LinxIpcTargets
    FILE LinxIpcTargets.cmake
    NAMESPACE LinxIpc::
    DESTINATION lib/cmake/LinxIpc
)

add_to_ut(TARGET LinxIpc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxMessageTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixClientTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixServerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixSocketTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxEventFdTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcHandlerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcIntegrationTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcPerformanceTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/DeadlineTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/UdpFactoryTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/UdpSocketTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/SylogEnvironment.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
