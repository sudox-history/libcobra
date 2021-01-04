#ifndef COBRA_SOCKET_H
#define COBRA_SOCKET_H

#include <stdbool.h>
#include <stdint.h>
#ifdef COBRA_SOCKET_PRIVATE
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "cobra/buffer.h"
#endif

#define COBRA_SOCKET_PACKET_HEADER_LENGTH 2u

#define COBRA_SOCKET_PACKET_MAX_LENGTH \
    ((int) ((1u << (8 * COBRA_SOCKET_PACKET_HEADER_LENGTH)) - 1))

#define COBRA_SOCKET_PACKET_BODY_MAX_LENGTH \
    COBRA_SOCKET_PACKET_MAX_LENGTH          \
    -COBRA_SOCKET_PACKET_HEADER_LENGTH

#define COBRA_SOCKET_OK 0
#define COBRA_SOCKET_ERR_NOT_CLOSED 1
#define COBRA_SOCKET_ERR_NOT_CONNECTED 2
#define COBRA_SOCKET_ERR_RESOLVING 3
#define COBRA_SOCKET_ERR_CONNECTING 4
#define COBRA_SOCKET_ERR_QUEUE_FULL 5
#define COBRA_SOCKET_ERR_QUEUE_OVERFLOW 6

#ifdef COBRA_SOCKET_PRIVATE
#define COBRA_SOCKET_STATUS_CLOSED 1
#define COBRA_SOCKET_STATUS_RESOLVING 2
#define COBRA_SOCKET_STATUS_CONNECTING 3
#define COBRA_SOCKET_STATUS_CONNECTED 4
#define COBRA_SOCKET_STATUS_CLOSING 5
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
    uv_mutex_t mutex_handle;

    uv_loop_t loop;
    uv_tcp_t tcp_handle;
    uv_timer_t timer_handle;
    uv_async_t async_handle;

    int connection_status;
    bool connection_alive;
};
#endif

/**
 * Connection
 */
int cobra_socket_connect(cobra_socket_t *sock, char *host, char *port);
#ifdef COBRA_SOCKET_PRIVATE
void cobra__socket_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo);
#endif

#endif//COBRA_SOCKET_H