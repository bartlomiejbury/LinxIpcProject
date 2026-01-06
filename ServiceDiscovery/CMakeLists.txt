add_library(ServiceDiscovery STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/ServiceDiscovery.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ServiceRepository.cpp
)

target_include_directories(
    ServiceDiscovery
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include/ServiceDiscovery>
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/
)

target_link_libraries(ServiceDiscovery
    PRIVATE
    LinxIpc
)

if(LINXIPC_INSTALL)
    install(TARGETS ServiceDiscovery
        EXPORT LinxIpcTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
    )

    # Install public headers
    install(DIRECTORY include/
        DESTINATION include/LinxIpc/ServiceDiscovery
        FILES_MATCHING PATTERN "*.h"
    )
endif()

add_executable(SdServer ${CMAKE_CURRENT_LIST_DIR}/src/Server.cpp)
target_link_libraries(SdServer PRIVATE ServiceDiscovery LinxIpc)

add_executable(TestApp ${CMAKE_CURRENT_LIST_DIR}/src/TestApp.cpp)
target_link_libraries(TestApp PRIVATE ServiceDiscovery LinxIpc)