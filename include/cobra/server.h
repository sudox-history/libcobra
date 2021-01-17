#ifndef COBRA_SERVER_H
#define COBRA_SERVER_H

#ifdef COBRA_SERVER_PRIVATE
#include <uv.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

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
    COBRA_SERVER_ERR_LISTENING
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

typedef void (*cobra_server_connection_cb)
        (cobra_server_t *server, cobra_socket_t *socket);

typedef void (*cobra_server_close_cb)
        (cobra_server_t *server, cobra_server_err_t error);

#ifdef COBRA_SERVER_PRIVATE
#define COBRA_SERVER_BACKLOG 128
struct cobra_server_t {
    uv_loop_t loop;
    uv_tcp_t tcp_handle;

    uv_thread_t thread_handle;
    uv_mutex_t mutex_handle;
    cobra_async_t close_async;

    uv_getaddrinfo_t *resolve_request;
    cobra__server_state_t state;

    int sockets_write_queue_size;

    cobra_server_connection_cb connection_callback;
    cobra_server_close_cb close_callback;
};
#endif

/**
 * Listening method
 */
cobra_server_err_t cobra_server_listen(cobra_server_t *server, char *host, char *port);
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


#endif //COBRA_SERVER_H
