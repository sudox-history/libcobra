cmake_minimum_required(VERSION 3.6)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
# v1.40.0
add_subdirectory(deps/libuv)

# libcobra
# Current library
set(COBRA_SOURCES src/buffer.c
        src/buffer.h
        src/tools.c
        src/tools.h
        src/socket.c
        src/socket.h
        src/server.c
        src/server.h
        src/discovery.c
        src/discovery.h)

add_library(cobra_a STATIC ${COBRA_SOURCES})
add_library(cobra SHARED ${COBRA_SOURCES})

target_include_directories(cobra_a
        PUBLIC include
        INTERFACE src)

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