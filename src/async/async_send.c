#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

cobra_async_err_t cobra_async_send(cobra_async_t *async, void *data) {
    uv_mutex_lock(&async->mutex_handle);

    if (cobra_queue_capacity(&async->data_queue) == 0) {
        uv_mutex_unlock(&async->mutex_handle);
        return COBRA_ASYNC_QUEUE_OVERFLOW;
    }

    cobra_queue_write(&async->data_queue, data);
    uv_async_send(&async->async_handle);

    bool full = cobra_queue_capacity(&async->data_queue) == 0;
    uv_mutex_unlock(&async->mutex_handle);

    return full ? COBRA_ASYNC_QUEUE_FULL : COBRA_ASYNC_OK;
}

void cobra__async_send_callback(uv_async_t *async_handle) {
    cobra_async_t *async = uv_handle_get_data((uv_handle_t *) async_handle);

    uv_mutex_lock(&async->mutex_handle);
    bool drain_flag = cobra_queue_capacity(&async->data_queue) == 0;

    if (async->send_callback != NULL)
        while (cobra_queue_length(&async->data_queue) != 0) {
            void *data = cobra_queue_read(&async->data_queue);
            uv_mutex_unlock(&async->mutex_handle);

            async->send_callback(async, data);
            uv_mutex_lock(&async->mutex_handle);
        }
    else
        cobra_queue_clear(&async->data_queue);

    uv_mutex_unlock(&async->mutex_handle);
    if (drain_flag && async->drain_callback != NULL)
        async->drain_callback(async);
}
