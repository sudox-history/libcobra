#ifndef COBRA_ASYNC_H
#define COBRA_ASYNC_H

#include <uv.h>
#include "cobra/queue.h"
#ifdef COBRA_ASYNC_PRIVATE
#include <stddef.h>
#endif

typedef enum {
    COBRA_ASYNC_OK,
    COBRA_ASYNC_QUEUE_FULL,
    COBRA_ASYNC_QUEUE_OVERFLOW
} cobra_async_err_t;

typedef struct cobra_async_t cobra_async_t;

typedef void (*cobra_async_send_cb)
        (cobra_async_t *async, void *data);

typedef void (*cobra_async_drain_cb)
        (cobra_async_t *async);

typedef void (*cobra_async_close_cb)
        (cobra_async_t *async);

struct cobra_async_t {
    uv_mutex_t mutex_handle;
    uv_async_t async_handle;
    cobra_queue_t data_queue;

    cobra_async_send_cb send_callback;
    cobra_async_drain_cb drain_callback;
    cobra_async_close_cb close_callback;

    void *data;
};

/**
 * Basic methods
 */
void cobra_async_init(cobra_async_t *async, uint64_t size);
void cobra_async_deinit(cobra_async_t *async);

/**
 * Binding methods
 */
void cobra_async_bind(cobra_async_t *async, uv_loop_t *loop);
void cobra_async_close(cobra_async_t *async);
#ifdef COBRA_ASYNC_PRIVATE
void cobra__async_close_callback(uv_handle_t *async_handle);
#endif

/**
 * Send method
 */
cobra_async_err_t cobra_async_send(cobra_async_t *async, void *data);
#ifdef COBRA_ASYNC_PRIVATE
void cobra__async_send_callback(uv_async_t *async_handle);
#endif

/**
 * Getters and setters
 */
void cobra_async_set_callbacks(cobra_async_t *async,
                               cobra_async_send_cb send_callback,
                               cobra_async_drain_cb drain_callback,
                               cobra_async_close_cb close_callback);

void cobra_async_set_data(cobra_async_t *async, void *data);
void *cobra_async_get_data(cobra_async_t *async);


#endif //COBRA_ASYNC_H
