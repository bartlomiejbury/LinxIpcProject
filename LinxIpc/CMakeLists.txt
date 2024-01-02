
add_library(LinxIpc
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcMessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcSocketImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcEndpointImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxQueueImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcClientImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxIpcServerImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/LinxTime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/trace.cpp
)

target_include_directories(
    LinxIpc
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/exports
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(LinxIpc pthread)

if(DEFINED USE_LOGGING)
    target_compile_definitions(LinxIpc PRIVATE USE_LOGGING=${USE_LOGGING})
endif()

add_to_ut(TARGET LinxIpc
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxTimeTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxQueueTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcMessageTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcClientTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcEndpointTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tests/LinxIpcServerTests.cpp
    MOCKS
    ${CMAKE_CURRENT_LIST_DIR}/tests/mocks
)
