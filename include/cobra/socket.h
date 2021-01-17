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
    COBRA_SOCKET_ERR_NOT_CONNECTED,
    COBRA_SOCKET_ERR_NOT_CLOSED,
    COBRA_SOCKET_ERR_RESOLVING,
    COBRA_SOCKET_ERR_CONNECTING,
    COBRA_SOCKET_ERR_WRONG_DATA,
    COBRA_SOCKET_ERR_QUEUE_FULL,
    COBRA_SOCKET_ERR_QUEUE_OVERFLOW,
    COBRA_SOCKET_ERR_UNKNOWN_WRITE_ERROR,
    COBRA_SOCKET_ERR_UNREACHABLE,
} cobra_socket_err_t;

#ifdef COBRA_SOCKET_PRIVATE
#define COBRA_SOCKET_PING_TIMEOUT 6000
#define COBRA_SOCKET_PING_CHECK_TIMEOUT 2000

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
        (cobra_socket_t *socket, cobra_socket_err_t error);

typedef void (*cobra_socket_alloc_cb)
        (cobra_socket_t *socket, uint8_t **data, uint64_t length);

typedef void (*cobra_socket_read_cb)
        (cobra_socket_t *socket, uint8_t *data, uint64_t length);

typedef void (*cobra_socket_write_cb)
        (cobra_socket_t *socket, uint8_t *data, uint64_t length, cobra_socket_err_t error);

typedef void (*cobra_socket_drain_cb)
        (cobra_socket_t *socket);

#ifdef COBRA_SOCKET_PRIVATE
#define COBRA_SOCKET_TOTAL_HANDLERS_COUNT 5
struct cobra_socket_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;
    uv_timer_t timer_handle;
    uv_timer_t check_timer_handle;

    uv_thread_t thread_handle;
    uv_mutex_t mutex_handle;
    cobra_async_t write_async;
    cobra_async_t close_async;

    uv_getaddrinfo_t *resolve_request;
    uv_connect_t *connect_request;
    cobra__socket_state_t state;
    cobra__socket_alive_t alive;

    cobra_socket_err_t close_error;
    int closed_handlers_count;

    cobra_buffer_t read_buffer;
    uint64_t read_frame_body_length;

    cobra_socket_connect_cb connect_callback;
    cobra_socket_close_cb close_callback;
    cobra_socket_alloc_cb alloc_callback;
    cobra_socket_read_cb read_callback;
    cobra_socket_write_cb write_callback;
    cobra_socket_drain_cb drain_callback;

    void *data;
};
#endif

/**
 * Base methods
 */
cobra_socket_t *cobra_socket_create(int write_queue_size);
cobra_socket_err_t cobra_socket_destroy(cobra_socket_t *sock);
#ifdef COBRA_SOCKET_SERVER_INTERNALS
void cobra__socket_bind(cobra_socket_t *sock, uv_loop_t *loop);
uv_tcp_t *cobra__socket_get_tcp_handle(cobra_socket_t *sock);
#endif

/**
 * Connection method
 */
cobra_socket_err_t cobra_socket_connect(cobra_socket_t *sock, char *host, char *port);
#ifdef COBRA_SOCKET_PRIVATE
typedef struct {
    cobra_socket_t *sock;
    char *host;
    char *port;
} cobra__socket_connect_ctx_t;

void cobra__socket_connect_thread(void *connect_context);

void cobra__socket_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo);

void cobra__socket_connect_callback(uv_connect_t *connect_request,
                                    int error);
#endif

/**
 * Closing method
 */
cobra_socket_err_t cobra_socket_close(cobra_socket_t *sock);
#ifdef COBRA_SOCKET_PRIVATE
cobra_socket_err_t cobra__socket_close(cobra_socket_t *sock, cobra_socket_err_t error);

typedef struct {
    cobra_socket_t *sock;
    cobra_socket_err_t error;
} cobra__socket_close_ctx_t;

void cobra__socket_close_async_send_callback(cobra_async_t *async, void *close_context);
void cobra__socket_async_close_callback(cobra_async_t *async);

void cobra__socket_close_callback(uv_handle_t *handle);
#endif

/**
 * Reading
 */
#if defined(COBRA_SOCKET_SERVER_INTERNALS) || defined(COBRA_SOCKET_PRIVATE)
void cobra__socket_start_read(cobra_socket_t *sock);
#endif

#ifdef COBRA_SOCKET_PRIVATE
void cobra__socket_alloc_callback(uv_handle_t *tcp_handle,
                                  size_t length,
                                  uv_buf_t *buffer);

void cobra__socket_read_callback(uv_stream_t *tcp_handle,
                                 ssize_t length,
                                 const uv_buf_t *buffer);
#endif

/**
 * Writing method
 */
cobra_socket_err_t cobra_socket_write(cobra_socket_t *sock, uint8_t *data, uint64_t length);
#ifdef COBRA_SOCKET_PRIVATE
cobra_socket_err_t cobra__socket_write(cobra_socket_t *sock, uint8_t *data, uint64_t length);

typedef struct {
    cobra_socket_t *sock;
    uint8_t *data;
    uint64_t length;
} cobra__socket_write_ctx_t;

typedef struct {
    cobra_socket_t *sock;
    uint8_t *data;
    uint64_t length;
    cobra_buffer_t frame;
    uv_write_t request;
} cobra__socket_uv_write_ctx_t;

void cobra__socket_write_async_send_callback(cobra_async_t *async, void *write_context);
void cobra__socket_write_async_drain_callback(cobra_async_t *async);

void cobra__socket_write_callback(uv_write_t *write_request, int error);

#endif

/**
 * Pinging
 */
#if defined(COBRA_SOCKET_SERVER_INTERNALS) || defined(COBRA_SOCKET_PRIVATE)
void cobra__socket_start_ping(cobra_socket_t *sock);
#endif

#ifdef COBRA_SOCKET_PRIVATE
void cobra__socket_ping_refresh_timer(cobra_socket_t *sock);
void cobra__socket_ping_frame(cobra_socket_t *sock);

void cobra__socket_timer_callback(uv_timer_t *timer_handle);
void cobra__socket_check_timer_callback(uv_timer_t *timer_handle);
#endif

/**
 * Getters and setters
 */
void cobra_socket_set_callbacks(cobra_socket_t *sock,
                                cobra_socket_connect_cb connect_callback,
                                cobra_socket_close_cb close_callback,
                                cobra_socket_alloc_cb alloc_callback,
                                cobra_socket_read_cb read_callback,
                                cobra_socket_write_cb write_callback,
                                cobra_socket_drain_cb drain_callback);

void cobra_socket_set_data(cobra_socket_t *sock, void *data);
void *cobra_socket_get_data(cobra_socket_t *sock);

#endif//COBRA_SOCKET_H