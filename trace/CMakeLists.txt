
add_library(trace STATIC
    ${CMAKE_CURRENT_LIST_DIR}/src/trace.cpp
)

target_include_directories(
    trace
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
    $<INSTALL_INTERFACE:include/LinxIpc/trace>
)

# Install trace library and headers
install(TARGETS trace
    EXPORT LinxIpcTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/LinxIpc/trace
    FILES_MATCHING PATTERN "*.h"
)
