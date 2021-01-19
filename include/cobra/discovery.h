#ifndef COBRA_DISCOVERY_H
#define CORBA_DISCOVERY_H

#ifdef COBRA_DISCOVERY_PRIVATE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>
#include "cobra/async.h"
#include "cobra/buffer.h"
#endif

typedef enum {
    COBRA_DISCOVERY_OK,
    COBRA_DISCOVERY_ERR_ALREADY_OPENED,
    COBRA_DISCOVERY_ERR_ALREADY_CLOSED,
    COBRA_DISCOVERY_ERR_BINDING,
    COBRA_DISCOVERY_ERR_JOINING_GROUP,
    COBRA_DISCOVERY_ERR_SENDING_FRAME,
    COBRA_DISCOVERY_ERR_NOT_CLOSED
} cobra_discovery_err_t;

#ifdef COBRA_DISCOVERY_PRIVATE
uint8_t COBRA_DISCOVERY_PACKET[] = {8, 100, 193, 210, 19};
#define COBRA_DISCOVERY_ANY_ADDR "0.0.0.0"
#define COBRA_DISCOVERY_MULTICAST_ADDR "239.255.255.250"
#define COBRA_DISCOVERY_PORT 55669
#define COBRA_DISCOVERY_TIMEOUT 5000
#define COBRA_DISCOVERY_HOST_STRLEN 16

typedef enum {
    COBRA_DISCOVERY_STATE_INITIALIZING,
    COBRA_DISCOVERY_STATE_LISTENING,
    COBRA_DISCOVERY_STATE_SCANNING,
    COBRA_DISCOVERY_STATE_CLOSING,
    COBRA_DISCOVERY_STATE_CLOSED
} cobra__discovery_state_t;
#endif

typedef struct cobra_discovery_t cobra_discovery_t;

typedef void (*cobra_discovery_found_cb)(cobra_discovery_t *discovery,
                                         char *host);

typedef void (*cobra_discovery_close_cb)(cobra_discovery_t *discovery,
                                         cobra_discovery_err_t error);

#ifdef COBRA_DISCOVERY_PRIVATE
#define COBRA_DISCOVERY_TOTAL_HANDLERS_COUNT 3
struct cobra_discovery_t {
    uv_loop_t loop;
    uv_udp_t udp_handle;
    uv_timer_t timer_handle;

    uv_mutex_t mutex_handle;
    uv_thread_t thread_handle;
    cobra_async_t close_async;

    cobra__discovery_state_t state;

    cobra_discovery_err_t close_error;
    int closed_handlers_count;

    cobra_buffer_t read_buffer;

    cobra_discovery_found_cb found_callback;
    cobra_discovery_close_cb close_callback;

    void *data;
};
#endif

/**
 * Base methods
 */
cobra_discovery_t *cobra_discovery_create();
void cobra_discovery_destroy(cobra_discovery_t *discovery);

/**
 * Listen method
 */
cobra_discovery_err_t cobra_discovery_listen(cobra_discovery_t *discovery);
#ifdef COBRA_DISCOVERY_PRIVATE
typedef struct {
    cobra_discovery_t *discovery;
} cobra__discovery_listen_ctx_t;

void cobra__discovery_listen_thread(void *listen_ctx);

void cobra__discovery_listener_alloc_callback(uv_handle_t *udp_handle,
                                              size_t length,
                                              uv_buf_t *buffer);

void cobra__discovery_listener_read_callback(uv_udp_t *udp_handle,
                                             ssize_t length,
                                             const uv_buf_t *buffer,
                                             const struct sockaddr *addr,
                                             unsigned _);

void cobra__discovery_listener_send_callback(uv_udp_send_t *send_request,
                                             int error);
#endif

/**
 * Scan method
 */
cobra_discovery_err_t cobra_discovery_scan(cobra_discovery_t *discovery);
#ifdef COBRA_DISCOVERY_PRIVATE
typedef struct {
    cobra_discovery_t *discovery;
} cobra__discovery_scan_ctx_t;

void cobra__discovery_scan_thread(void *scan_ctx);

void cobra__discovery_scanner_timer_callback(uv_timer_t *timer_handle);

void cobra__discovery_scanner_read_callback(uv_udp_t *udp_handle,
                                            ssize_t length,
                                            const uv_buf_t *buffer,
                                            const struct sockaddr *addr,
                                            unsigned _);

#endif

/**
 * Closing
 */
cobra_discovery_err_t cobra_discovery_close(cobra_discovery_t *discovery);
#ifdef COBRA_DISCOVERY_PRIVATE
typedef struct {
    cobra_discovery_t *discovery;
    cobra_discovery_err_t error;
} cobra__discovery_close_ctx_t;

cobra_discovery_err_t cobra__discovery_close(cobra_discovery_t *discovery,
                                             cobra_discovery_err_t error);

void cobra__discovery_close_async_send_callback(cobra_async_t *async,
                                                void *close_ctx);

void cobra__discovery_close_callback(uv_handle_t *handle);
void cobra__discovery_async_close_callback(cobra_async_t *async);

#endif

/**
 * Getters and setters
 */
void cobra_discovery_set_callbacks(cobra_discovery_t *discovery,
                                   cobra_discovery_found_cb found_callback,
                                   cobra_discovery_close_cb close_callback);

void cobra_discovery_set_data(cobra_discovery_t *discovery, void *data);
void *cobra_discovery_get_data(cobra_discovery_t *discovery);

#endif  // COBRA_DISCOVERY_H