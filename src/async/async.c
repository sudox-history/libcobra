#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_init(cobra_async_t *async, uint64_t size) {
    uv_mutex_init(&async->mutex_handle);
    uv_handle_set_data((uv_handle_t *)&async->async_handle, async);
    cobra_queue_init(&async->data_queue, size);
}

void cobra_async_deinit(cobra_async_t *async) {
    cobra_queue_deinit(&async->data_queue);
}