#ifndef COBRA_SERVER_H
#define COBRA_SERVER_H

#ifdef COBRA_SERVER_PRIVATE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <uv.h>

#include "cobra/async.h"
#define COBRA_SOCKET_SERVER_INTERNALS
#include "cobra/socket.h"
#endif

typedef enum {
    COBRA_SERVER_OK,
    COBRA_SERVER_ERR_ALREADY_OPENED,
    COBRA_SERVER_ERR_ALREADY_CLOSED,
    COBRA_SERVER_ERR_RESOLVING,
    COBRA_SERVER_ERR_BINDING,
    COBRA_SERVER_ERR_LISTENING,
    COBRA_SERVER_ERR_NOT_CLOSED
} cobra_server_err_t;

#ifdef COBRA_SERVER_PRIVATE
typedef enum {
    COBRA_SERVER_STATE_INITIALIZING = 1,
    COBRA_SERVER_STATE_RESOLVING,
    COBRA_SERVER_STATE_LISTENING,
    COBRA_SERVER_STATE_CLOSING,
    COBRA_SERVER_STATE_CLOSED
} cobra__server_state_t;
#endif

typedef struct cobra_server_t cobra_server_t;

typedef void (*cobra_server_connection_cb)(cobra_server_t *server,
                                           cobra_socket_t *socket);

typedef void (*cobra_server_close_cb)(cobra_server_t *server,
                                      cobra_server_err_t error);

#ifdef COBRA_SERVER_PRIVATE
#define COBRA_SERVER_BACKLOG 128
#define COBRA_SERVER_TOTAL_HANDLERS_COUNT 2
struct cobra_server_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;

    uv_thread_t thread_handle;
    uv_mutex_t mutex_handle;
    cobra_async_t close_async;

    uv_getaddrinfo_t *resolve_request;
    cobra__server_state_t state;

    cobra_server_err_t close_error;
    int closed_handlers_count;
    int sockets_write_queue_size;

    cobra_server_connection_cb connection_callback;
    cobra_server_close_cb close_callback;

    void *data;
};
#endif

/**
 * Base methods
 */
cobra_server_t *cobra_server_create(int sockets_write_queue_size);
cobra_server_err_t cobra_server_destroy(cobra_server_t *server);

/**
 * Listening method
 */
cobra_server_err_t cobra_server_listen(cobra_server_t *server,
                                       char *host,
                                       char *port);
#ifdef COBRA_SERVER_PRIVATE
typedef struct {
    cobra_server_t *server;
    char *host;
    char *port;
} cobra__server_listen_ctx_t;

void cobra_server_listen_thread(void *listen_ctx);

void cobra__server_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo);

void cobra__server_connection_callback(uv_stream_t *tcp_handle, int error);
#endif

/**
 * Closing method
 */
cobra_server_err_t cobra_server_close(cobra_server_t *server);
#ifdef COBRA_SERVER_PRIVATE
cobra_server_err_t cobra__server_close(cobra_server_t *server,
                                       cobra_server_err_t error);

typedef struct {
    cobra_server_t *server;
    cobra_server_err_t error;
} cobra__server_close_ctx_t;

void cobra__server_close_async_send_callback(cobra_async_t *async, void *data);

void cobra__server_close_callback(uv_handle_t *handle);
void cobra__server_async_close_callback(cobra_async_t *async);
#endif

/**
 * Getters and setters
 */
void cobra_server_set_callbacks(cobra_server_t *server,
                                cobra_server_connection_cb connection_callback,
                                cobra_server_close_cb close_callback);

#endif  // COBRA_SERVER_H
