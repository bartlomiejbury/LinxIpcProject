
add_option(VAR LINX_LOG_PREFIX)

add_library(LinxIpc STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/message/LinxMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/LinxIpcHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixSocket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixEndpoint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/unix/AfUnixServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxQueue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxEventFd.cpp
)

target_include_directories(
    LinxIpc
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/unix
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/message
    ${CMAKE_CURRENT_LIST_DIR}/src/unix
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
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
    ${CMAKE_CURRENT_LIST_DIR}/tests/AfUnixEndpointTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxEventFdTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcHandlerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcIntegrationTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcPerformanceTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/DeadlineTests.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
