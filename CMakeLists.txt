cmake_minimum_required(VERSION 3.18.3)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
add_subdirectory(deps/libuv)

# libcobra
include_directories(include)
add_library(cobra
        src/internal/buffer.c
        src/internal/buffer.h
        src/internal/tcp_connection.c
        src/internal/tcp_connection.h
        src/internal/discovery.c
        src/internal/discovery.h)

# linking libuv to libcobra
target_link_libraries(cobra uv_a)

add_executable(main src/main.c)
target_link_libraries(main cobra)