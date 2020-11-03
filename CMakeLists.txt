cmake_minimum_required(VERSION 3.18.3)
project(libcobra C)

set(CMAKE_C_STANDARD 11)

# libuv
add_subdirectory(deps/libuv)

add_executable(main src/main.c src/transport_connection.c src/transport_connection.h include/cobra.h include/cobra/errors.h src/transport_constants.h src/transport_buffer.c src/transport_buffer.h)
target_link_libraries(main uv_a)