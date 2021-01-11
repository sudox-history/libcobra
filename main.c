#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cobra/async.h"

void on_connect(uv_connect_t* req, int status) {
    printf("From connect: %s\n", uv_strerror(status));
    fflush(stdout);
}

int main() {
    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_tcp_t tcp_handle;
    uv_tcp_init(&loop, &tcp_handle);

    struct sockaddr addr;
    uv_ip4_addr("192.168.1.4", 5000, &addr);

    uv_connect_t connect_request;
    int r = uv_tcp_connect(&connect_request, &tcp_handle, &addr, on_connect);
    printf("Result: %s\n", uv_strerror(r));
    fflush(stdout);

    uv_run(&loop, UV_RUN_DEFAULT);

    uv_loop_close(&loop);
    printf("Hello world\n");
}