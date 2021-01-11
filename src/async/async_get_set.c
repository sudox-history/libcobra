#define COBRA_ASYNC_PRIVATE
#include "cobra/async.h"

void cobra_async_set_callbacks(cobra_async_t *async,
                               cobra_async_send_cb send_callback,
                               cobra_async_drain_cb drain_callback,
                               cobra_async_drain_cb close_callback) {
    async->send_callback = send_callback;
    async->drain_callback = drain_callback;
    async->close_callback = close_callback;
}


void cobra_async_set_data(cobra_async_t *async, void *data) {
    async->data = data;
}

void *cobra_async_get_data(cobra_async_t *async) {
    return async->data;
}