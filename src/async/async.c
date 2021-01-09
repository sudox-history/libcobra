#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_init(cobra_async_t *async, uv_loop_t *loop, uint64_t size) {
    uv_async_init(loop, &async->async_handle, cobra__async_main_callback);
    uv_handle_set_data((uv_handle_t *) &async->async_handle, async);
    cobra_queue_init(&async->data_queue, size);
}

void cobra_async_deinit(cobra_async_t *async) {
    // Don't care about close callback, because we don't need to free any resources
    uv_close((uv_handle_t *) &async->async_handle, NULL);
    cobra_queue_deinit(&async->data_queue);
}

void cobra__async_main_callback(uv_async_t *async_handle) {
    cobra_async_t *async = uv_handle_get_data((uv_handle_t *) async_handle);

    bool drain_flag = cobra_queue_capacity(&async->data_queue) == 0;

    while (cobra_queue_length(&async->data_queue) != 0) {
        async->main_callback(async, cobra_queue_read(&async->data_queue));
    }

    if (drain_flag && async->drain_callback != NULL)
        async->drain_callback(async);
}