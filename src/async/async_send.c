#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

int cobra_async_send(cobra_async_t *async, void *data) {
    if (cobra_queue_capacity(&async->data_queue) == 0)
        return COBRA_ASYNC_QUEUE_OVERFLOW;

    cobra_queue_write(&async->data_queue, data);
    uv_async_send(&async->async_handle);

    if (cobra_queue_capacity(&async->data_queue) == 0)
        return COBRA_ASYNC_QUEUE_FULL;

    return COBRA_ASYNC_OK;
}
