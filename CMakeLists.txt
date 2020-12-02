cmake_minimum_required(VERSION 3.10)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
# v1.40.0
add_subdirectory(deps/libuv)

# libcobra
# Current library
add_library(cobra_a
        src/models/buffer.c
        src/models/buffer.h
        src/misc/utils.c
        src/misc/utils.h
        src/core/socket.c
        src/core/socket.h
        src/core/server.c
        src/core/server.h
        src/core/discovery.c
        src/core/discovery.h src/models/queue.c src/models/queue.h src/misc/platform.c src/misc/platform.h)

target_include_directories(cobra_a
        PUBLIC include
        INTERFACE src
        PRIVATE src)

target_link_libraries(cobra_a
        PUBLIC uv_a)


# Main
# Used for fast testing during development
add_executable(main
        src/main.c)

target_link_libraries(main
        cobra_a)

target_link_options(main
        PUBLIC -fsanitize=address,undefined)