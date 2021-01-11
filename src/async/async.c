#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_init(cobra_async_t *async, uint64_t size) {
    cobra_queue_init(&async->data_queue, size);
}

void cobra_async_deinit(cobra_async_t *async) {
    cobra_queue_deinit(&async->data_queue);
}

void cobra_async_bind(cobra_async_t *async, uv_loop_t *loop) {
    uv_async_init(loop, &async->async_handle, cobra__async_send_callback);
    uv_handle_set_data((uv_handle_t *) &async->async_handle, async);
}

void cobra_async_unbind(cobra_async_t *async) {
    // Don't care about close callback, because we don't need to free any resources
    uv_close((uv_handle_t *) &async->async_handle, NULL);
}