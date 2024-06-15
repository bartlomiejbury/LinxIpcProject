
add_library(trace OBJECT
    ${CMAKE_CURRENT_LIST_DIR}/src/trace.cpp
)

target_include_directories(
    trace
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/exports
)

add_to_ut(TARGET trace)
