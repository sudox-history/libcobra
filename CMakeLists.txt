cmake_minimum_required(VERSION 3.10)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
# v1.40.0
add_subdirectory(deps/libuv)

# libcobra
# Current library
add_library(cobra_a
        src/internal/buffer.c
        src/internal/buffer.h
        src/internal/tools.c
        src/internal/tools.h
        src/socket.c
        include/cobra/socket.h
        src/server.c
        include/cobra/server.h
        src/discovery.c
        include/cobra/discovery.h src/internal/queue.c src/internal/queue.h)

target_include_directories(cobra_a
        PUBLIC include
        INTERFACE src
        PRIVATE src)

target_link_libraries(cobra_a
        PUBLIC uv_a)


# Main
# Used for fast testing during development
#add_executable(main
#        src/main.c)
#
#target_link_libraries(main
#        cobra_a)
#
#target_link_options(main
#        PUBLIC -fsanitize=address,undefined)