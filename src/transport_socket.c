#define COBRA_TRANSPORT_SOCKET_PRIVATE
#include "transport_socket.h"

cobra_transport_socket_t *cobra_transport_socket_create() {
    cobra_transport_socket_t *transport_socket = malloc(sizeof(cobra_transport_socket_t));

    /* Initializing libuv structs */
    uv_loop_init(&transport_socket->loop);
    uv_tcp_init(&transport_socket->loop, &transport_socket->tcp_handle);

    /* Initializing other members */
    transport_socket->connected = false;
    transport_socket->read_buffer = cobra_buffer_create(CORBA_PACKET_MAX_SIZE);
}