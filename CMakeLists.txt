cmake_minimum_required(VERSION 3.14)
project(LinxIpc VERSION 1.0.0 LANGUAGES CXX)

add_compile_options(
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>
    -Wall -Wno-psabi -ffunction-sections -fdata-sections -fno-common -g -fstack-protector-strong -Werror -Og
)

add_link_options(
    -Wl,-Map=output.map
    -Wl,--gc-sections
)

add_subdirectory(LinxIpc)
add_subdirectory(TestApp)
