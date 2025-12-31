
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
)

target_include_directories(
    LinxIpc
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/common
    ${CMAKE_CURRENT_LIST_DIR}/include/generic
    ${CMAKE_CURRENT_LIST_DIR}/include/message
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/
    ${CMAKE_CURRENT_LIST_DIR}/src/message
    ${CMAKE_CURRENT_LIST_DIR}/src/unix
    ${CMAKE_CURRENT_LIST_DIR}/src/udp
    ${CMAKE_CURRENT_LIST_DIR}/src/common
    ${CMAKE_CURRENT_LIST_DIR}/src/queue
)

target_link_libraries(LinxIpc
    PUBLIC
    trace
)

add_to_ut(TARGET LinxIpc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxMessageTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixClientTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixServerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxEventFdTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcHandlerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcIntegrationTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcPerformanceTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/DeadlineTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/UdpFactoryTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/SylogEnvironment.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
