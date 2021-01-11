#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

int cobra_async_send(cobra_async_t *async, void *data) {
    cobra_queue_lock(&async->data_queue);

    if (cobra_queue_capacity(&async->data_queue) == 0) {
        cobra_queue_unlock(&async->data_queue);
        return COBRA_ASYNC_QUEUE_OVERFLOW;
    }

    cobra_queue_write(&async->data_queue, data);
    uv_async_send(&async->async_handle);

    bool full = cobra_queue_capacity(&async->data_queue) == 0;
    cobra_queue_unlock(&async->data_queue);

    return full ? COBRA_ASYNC_QUEUE_FULL : COBRA_ASYNC_OK;
}

void cobra__async_send_callback(uv_async_t *async_handle) {
    cobra_async_t *async = uv_handle_get_data((uv_handle_t *) async_handle);

    cobra_queue_lock(&async->data_queue);
    bool drain_flag = cobra_queue_capacity(&async->data_queue) == 0;

    while (cobra_queue_length(&async->data_queue) != 0) {
        async->main_callback(async, cobra_queue_read(&async->data_queue));
    }

    cobra_queue_unlock(&async->data_queue);
    if (drain_flag && async->drain_callback != NULL)
        async->drain_callback(async);
}
