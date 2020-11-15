#ifndef COBRA_SOCKET_H
#define COBRA_SOCKET_H

#include <stdint.h>
#include <stdbool.h>
#ifdef COBRA_SOCKET_PRIVATE
#include <uv.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#endif

#define COBRA_SOCKET_PACKET_HEADER_LENGTH 2u
#define COBRA_SOCKET_PACKET_MAX_SIZE \
    (int) ((1u << (8 * COBRA_SOCKET_PACKET_HEADER_LENGTH)) - 1)

#define COBRA_SOCKET_OK 0
#define COBRA_SOCKET_ERR_ALREADY_CONNECTED 1
#define COBRA_SOCKET_ERR_NOT_CONNECTED 2
#define COBRA_SOCKET_ERR_RESOLVING 3
#define COBRA_SOCKET_ERR_CONNECTING 4

typedef struct cobra_socket_t cobra_socket_t;

typedef void (*cobra_socket_connect_cb)
        (cobra_socket_t *socket);

typedef void (*cobra_socket_close_cb)
        (cobra_socket_t *socket, int error);

typedef void (*cobra_socket_alloc_cb)
        (cobra_socket_t *socket, uint8_t **data, uint64_t length);

typedef void (*cobra_socket_data_cb)
        (cobra_socket_t *socket, uint8_t *data, uint64_t length);

typedef void (*cobra_socket_drain_cb)
        (cobra_socket_t *socket);

#ifdef COBRA_SOCKET_PRIVATE
struct cobra_socket_t {
    /* Libuv handles */
    uv_loop_t loop;
    uv_tcp_t tcp_handle;

    /* Connection management */
    bool connected;
    int close_error;
    char *host;
    char *port;

    /* Write management */
    int write_queue_size;
    int write_queue_length;

    /* Read management */
    cobra_buffer_t *read_buffer;
    uint64_t read_packet_body_length;

    /* Callbacks */
    cobra_socket_connect_cb on_connect;
    cobra_socket_close_cb on_close;
    cobra_socket_alloc_cb on_alloc;
    cobra_socket_data_cb on_data;

    /* Additional user data */
    void *data;
};
#endif

cobra_socket_t *cobra_socket_create(int write_queue_size);
void cobra_socket_destroy(cobra_socket_t *socket);

int cobra_socket_connect(cobra_socket_t *socket, char *host, char *port);
int cobra_socket_close(cobra_socket_t *socket);

int cobra_socket_send(cobra_socket_t *socket, uint8_t *data, uint64_t length);

#endif //COBRA_SOCKET_H
