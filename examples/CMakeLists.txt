add_executable(client_app client-server-example/client.cpp)
target_link_libraries(client_app PRIVATE LinxIpc)

add_executable(server_app client-server-example/server.cpp)
target_link_libraries(server_app PRIVATE LinxIpc)

add_executable(simple_example package-example/simple_example.cpp)
target_link_libraries(simple_example PRIVATE LinxIpc)