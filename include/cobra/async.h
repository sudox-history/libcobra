#ifndef COBRA_ASYNC_H
#define COBRA_ASYNC_H

#include <uv.h>
#include "cobra/queue.h"
#ifdef COBRA_ASYNC_PRIVATE
#include <stddef.h>
#endif

typedef struct cobra_async_t cobra_async_t;

typedef void (*cobra_async_main_callback)
        (cobra_async_t *async, void *data);

struct cobra_async_t {
    uv_async_t async_handle;
    cobra_queue_t data_queue;
    cobra_async_main_callback main_callback;
};

/**
 * Basic methods
 */
void cobra_async_init(cobra_async_t *async, uv_loop_t *loop, uint64_t size);
void cobra_async_deinit(cobra_async_t *async);
#ifdef COBRA_ASYNC_PRIVATE
void cobra__async_callback(uv_async_t *async_handle);
#endif

/**
 * Send method
 */
void cobra_async_send(cobra_async_t *async, void *data);

/**
 * Callbacks
 */
void cobra_async_set_callbacks(cobra_async_t *async, cobra_async_main_callback main_callback);

#endif //COBRA_ASYNC_H
