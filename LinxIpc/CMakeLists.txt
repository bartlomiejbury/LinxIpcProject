
add_option(VAR LINX_LOG_PREFIX)

add_library(LinxIpc STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcSocketImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcServerImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxQueueImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxQueueFdImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcClientImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxTime.cpp
)

target_include_directories(
    LinxIpc
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(LinxIpc
    PUBLIC
    pthread
    trace
)

add_to_ut(TARGET LinxIpc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxTimeTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcMessageTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcClientTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcServerTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcEventFdTests.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
