#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

cobra_socket_t *cobra_socket_create(int write_queue_size) {
    cobra_socket_t *socket = malloc(sizeof(cobra_socket_t));

    uv_loop_init(&socket->loop);
    uv_tcp_init(&socket->loop, &socket->tcp_handle);

    socket->is_connected = false;
    socket->is_alive = false;
    socket->is_closing = false;
    socket->close_error = COBRA_SOCKET_OK;

    socket->write_queue_size = write_queue_size;
    socket->write_queue_length = 0;

    cobra_buffer_init(&socket->read_buffer, COBRA_SOCKET_PACKET_MAX_LENGTH);
    socket->read_packet_body_length = 0;

    socket->on_connect = NULL;
    socket->on_close = NULL;
    socket->on_alloc = NULL;
    socket->on_read = NULL;
    socket->on_drain = NULL;

    socket->data = NULL;
    return socket;
}

void cobra_socket_destroy(cobra_socket_t *socket) {
    if (socket->is_connected)
        cobra_socket_close(socket);

    cobra_buffer_deinit(&socket->read_buffer);
    free(socket);
}
