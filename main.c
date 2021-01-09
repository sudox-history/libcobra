#include <stdio.h>
#include <stdlib.h>
#include "cobra/async.h"

void main_callback(cobra_async_t *async, void *data) {
    int value = (int) data;
    printf("%d\n", value);
}

void drain_callback(cobra_async_t *async) {
    printf("DRAIN\n");
}

void another_thread(void *data) {
    cobra_async_t *async = data;

    cobra_async_send(async, (void *) 10);
    cobra_async_send(async, (void *) 11);
    int res = cobra_async_send(async, (void *) 12);

    printf("Res: %d\n", res);
}

int main() {
    uv_loop_t loop;
    uv_loop_init(&loop);

    cobra_async_t async;
    cobra_async_init(&async, &loop, 3);
    cobra_async_set_callbacks(&async, main_callback, drain_callback);

    uv_thread_t thread;
    uv_thread_create(&thread, another_thread, &async);

    uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);
    printf("Hello world\n");
}