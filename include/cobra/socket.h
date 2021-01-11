#ifndef COBRA_SOCKET_H
#define COBRA_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#ifdef COBRA_SOCKET_PRIVATE
#include "cobra/async.h"
#include "cobra/buffer.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#endif

#define COBRA_SOCKET_FRAME_HEADER_LENGTH 2u

#define COBRA_SOCKET_FRAME_BODY_MAX_LENGTH \
    ((int) ((1u << (8 * COBRA_SOCKET_FRAME_HEADER_LENGTH)) - 1))

#define COBRA_SOCKET_FRAME_MAX_LENGTH \
    COBRA_SOCKET_FRAME_BODY_MAX_LENGTH + COBRA_SOCKET_FRAME_HEADER_LENGTH

typedef enum {
    COBRA_SOCKET_OK,
    COBRA_SOCKET_ERR_ALREADY_OPENED,
    COBRA_SOCKET_ERR_ALREADY_CLOSED,
    COBRA_SOCKET_ERR_RESOLVING,
    COBRA_SOCKET_ERR_CONNECTING,
    COBRA_SOCKET_ERR_QUEUE_FULL,
    COBRA_SOCKET_ERR_QUEUE_OVERFLOW
} cobra_socket_err_t;

#ifdef COBRA_SOCKET_PRIVATE
typedef enum {
    COBRA_SOCKET_STATE_INITIALIZING = 1,
    COBRA_SOCKET_STATE_RESOLVING,
    COBRA_SOCKET_STATE_CONNECTING,
    COBRA_SOCKET_STATE_CONNECTED,
    COBRA_SOCKET_STATE_CLOSING,
    COBRA_SOCKET_STATE_CLOSED
} cobra__socket_state_t;

typedef enum {
    COBRA_SOCKET_ALIVE_OK = 1,
    COBRA_SOCKET_ALIVE_PING_SENT
} cobra__socket_alive_t;
#endif

typedef struct cobra_socket_t cobra_socket_t;

typedef void (*cobra_socket_connect_cb)
        (cobra_socket_t *socket);

typedef void (*cobra_socket_close_cb)
        (cobra_socket_t *socket, int error);

typedef void (*cobra_socket_alloc_cb)
        (cobra_socket_t *socket, uint8_t **data, uint64_t length);

typedef void (*cobra_socket_read_cb)
        (cobra_socket_t *socket, uint8_t *data, uint64_t length);

typedef void (*cobra_socket_drain_cb)
        (cobra_socket_t *socket);

#ifdef COBRA_SOCKET_PRIVATE
struct cobra_socket_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;
    uv_timer_t timer_handle;

    uv_thread_t thread_handle;
    uv_mutex_t mutex_handle;
    cobra_async_t write_async;
    cobra_async_t close_async;

    uv_getaddrinfo_t *resolve_request;
    uv_connect_t *connect_request;

    cobra__socket_state_t state;
    cobra__socket_alive_t alive;

    cobra_buffer_t read_buffer;
    uint64_t read_frame_body_length;

    cobra_socket_connect_cb connect_callback;
    cobra_socket_close_cb close_callback;
    cobra_socket_alloc_cb alloc_callback;
    cobra_socket_read_cb read_callback;
    cobra_socket_drain_cb drain_callback;
};
#endif

/**
 * Base methods
 */
cobra_socket_t *cobra_socket_create(int write_queue_size);
void cobra_socket_destroy(cobra_socket_t *sock);


/**
 * Connection method
 */
int cobra_socket_connect(cobra_socket_t *sock, char *host, char *port);
#ifdef COBRA_SOCKET_PRIVATE
typedef struct {
    cobra_socket_t *sock;
    char *host;
    char *port;
} cobra__socket_connect_ctx_t;

void cobra__socket_connect(void *connect_request);

void cobra__socket_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo);

void cobra__socket_connect_callback(uv_connect_t *connect_request,
                                    int error);
#endif

/**
 * Closing method
 */
int cobra_socket_close(cobra_socket_t *sock);
#ifdef COBRA_SOCKET_PRIVATE
int cobra__socket_close(cobra_socket_t *sock, int error);

typedef struct {
    cobra_socket_t *sock;
    int error;
} cobra__socket_close_ctx_t;

void cobra__socket_close_async_main_callback(cobra_async_t *async, void *close_request);
void cobra__socket_close_async_close_callback(cobra_async_t *async);

void cobra__socket_close_callback(uv_handle_t *tcp_handle);
#endif

/**
 * Reading methods
 */
#ifdef COBRA_SOCKET_PRIVATE
void cobra__socket_alloc_callback(uv_handle_t *tcp_handle,
                                  size_t length,
                                  uv_buf_t *buffer);

void cobra__socket_read_callback(uv_stream_t *tcp_handle,
                                 ssize_t length,
                                 const uv_buf_t *buffer);
#endif

#endif//COBRA_SOCKET_H