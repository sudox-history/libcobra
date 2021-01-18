#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_bind(cobra_async_t *async, uv_loop_t *loop) {
    uv_async_init(loop, &async->async_handle, cobra__async_send_callback);
    cobra_queue_clear(&async->data_queue);
}

void cobra_async_close(cobra_async_t *async) {
    uv_close((uv_handle_t *)&async->async_handle, cobra__async_close_callback);
}

void cobra__async_close_callback(uv_handle_t *async_handle) {
    cobra_async_t *async = uv_handle_get_data(async_handle);

    if (async->close_callback != NULL)
        async->close_callback(async);
}