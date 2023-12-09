add_executable(testClient1 EXCLUDE_FROM_ALL
    TestClient1.cpp
)

add_executable(testClient2 EXCLUDE_FROM_ALL
    TestClient2.cpp
)

target_link_libraries(testClient1 LinxIpc)
target_link_libraries(testClient2 LinxIpc)
