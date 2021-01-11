cmake_minimum_required(VERSION 3.10)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
# v1.40.0
add_subdirectory(deps/libuv)

# libcobra
# Current library
add_library(cobra_a
        src/async/async.c
        src/async/async_send.c
        src/async/async_bind.c
        src/async/async_get_set.c
        src/buffer/buffer.c
        src/buffer/buffer_sync.c
        src/buffer/buffer_write.c
        src/buffer/buffer_read.c
        src/buffer/buffer_memory.c
        src/queue/queue.c
        src/queue/queue_memory.c
        src/queue/queue_sync.c
        src/queue/queue_read.c
        src/queue/queue_write.c
        src/utils.c
        #src/socket/socket.c
        src/socket/socket_close.c
        src/socket/socket_connect.c
        #src/socket/socket_ping.c
        #src/socket/socket_read.c
        #src/socket/socket_write.c
        #src/server.c
        src/discovery.c
        src/platform.c)

target_include_directories(cobra_a
        PUBLIC include)

target_link_libraries(cobra_a
        PUBLIC uv_a)


# Main
# Used for fast testing during development
add_executable(main
        main.c)

target_link_libraries(main
        cobra_a)

target_link_options(main
        PUBLIC -fsanitize=address,undefined)