#ifndef COBRA_TRANSPORT_SOCKET_H
#define COBRA_TRANSPORT_SOCKET_H

#include <stdint.h>
#include <stdbool.h>
#ifdef COBRA_TRANSPORT_SOCKET_PRIVATE
#include <uv.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#endif


#ifdef COBRA_TRANSPORT_SOCKET_PRIVATE
#define COBRA_PACKET_HEADER_LENGTH 2u
#define CORBA_PACKET_MAX_SIZE (int) ((1u << (8 * COBRA_PACKET_HEADER_LENGTH)) - 1)

#endif

typedef struct cobra_transport_socket_t cobra_transport_socket_t;

#ifdef COBRA_TRANSPORT_SOCKET_PRIVATE
struct cobra_transport_socket_t {
    /* Libuv handles */
    uv_loop_t loop;
    uv_tcp_t tcp_handle;

    /* Connection management */
    bool connected;

    /* Read management */
    cobra_buffer_t *read_buffer;
    uint64_t read_packet_length;

    /* Callbacks */

};
#endif

cobra_transport_socket_t *cobra_transport_socket_create();
void cobra_transport_socket_destroy(cobra_transport_socket_t *transport_socket);

#endif //COBRA_TRANSPORT_SOCKET_H
