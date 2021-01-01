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
#define COBRA_SOCKET_ERR_ALREADY_CONNECTED 1
#define COBRA_SOCKET_ERR_NOT_CONNECTED 2
#define COBRA_SOCKET_ERR_RESOLVING 3
#define COBRA_SOCKET_ERR_CONNECTING 4
#define COBRA_SOCKET_ERR_QUEUE_FULL 5
#define COBRA_SOCKET_ERR_QUEUE_OVERFLOW 6

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

    bool is_connected;
    bool is_alive;
    bool is_closing;
    int close_error;

    cobra_buffer_t read_buffer;
    uint64_t read_packet_body_length;

    int write_queue_size;
    int write_queue_length;

    cobra_socket_connect_cb on_connect;
    cobra_socket_close_cb on_close;
    cobra_socket_alloc_cb on_alloc;
    cobra_socket_read_cb on_read;
    cobra_socket_drain_cb on_drain;

    void *data;
};
#endif

int cobra_socket_connect(cobra_socket_t *sock, char *host, char *port);

#endif//COBRA_SOCKET_H