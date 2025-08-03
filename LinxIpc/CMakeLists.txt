
add_option(VAR LINX_LOG_PREFIX)

add_library(LinxIpc STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/handler/LinxIpcHandlerImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxQueueImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/queue/LinxIpcEventFdImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/server/LinxIpcServerImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/endpoint/LinxIpcSocketImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/endpoint/LinxIpcEndpointImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/client/LinxIpcClientImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/LinxIpcMessage.cpp
)

target_include_directories(
    LinxIpc
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/utils
    ${CMAKE_CURRENT_LIST_DIR}/src/client
    ${CMAKE_CURRENT_LIST_DIR}/src/endpoint
    ${CMAKE_CURRENT_LIST_DIR}/src/server
    ${CMAKE_CURRENT_LIST_DIR}/src/handler
    ${CMAKE_CURRENT_LIST_DIR}/src/queue
)

target_link_libraries(LinxIpc
    PUBLIC
    pthread
    trace
)

add_to_ut(TARGET LinxIpc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcMessageTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcClientTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcEndpointTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcServerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcEventFdTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcHandlerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcIntegrationTests.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
