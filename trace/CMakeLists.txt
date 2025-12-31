
add_library(trace OBJECT
    ${CMAKE_CURRENT_LIST_DIR}/src/trace.cpp
)

target_include_directories(
    trace
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/exports>
    $<INSTALL_INTERFACE:include/LinxIpc/trace>
)

# Install trace headers
install(DIRECTORY exports/
    DESTINATION include/LinxIpc/trace
    FILES_MATCHING PATTERN "*.h"
)
