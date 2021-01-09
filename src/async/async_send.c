#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_send(cobra_async_t *async, void *data) {
    cobra_queue_write(&async->data_queue, data);
    uv_async_send(&async->async_handle);
}
