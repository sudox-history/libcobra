cmake_minimum_required(VERSION 3.18.3)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
add_subdirectory(deps/libuv)

add_executable(main src/main.c)
target_link_libraries(main uv_a)