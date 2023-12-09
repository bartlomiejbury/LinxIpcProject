add_executable(testClient1
    TestClient1.cpp
)

add_executable(testClient2
    TestClient2.cpp
)

target_link_libraries(testClient1 LinxIpc)
target_link_libraries(testClient2 LinxIpc)
