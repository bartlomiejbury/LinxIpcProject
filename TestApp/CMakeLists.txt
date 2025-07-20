add_executable(testClient1 EXCLUDE_FROM_ALL
    TestClient1.cpp
)

add_executable(ipcSender EXCLUDE_FROM_ALL
    ipcSender.cpp
)

add_executable(testClient3 EXCLUDE_FROM_ALL
    TestClient3.cpp
)

target_link_libraries(testClient1 LinxIpc trace)
target_link_libraries(ipcSender LinxIpc trace)
target_link_libraries(testClient3 LinxIpc trace)