#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

int cobra_socket_close(cobra_socket_t *sock) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->connection_status == COBRA_SOCKET_STATUS_CLOSED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_NOT_CONNECTED;
    }

    uv_mutex_unlock(&sock->mutex_handle);

    cobra__socket_close_request *close_request = malloc(sizeof(cobra__socket_close_request));
    close_request->sock = sock;

    cobra_queue_write(&sock->close_queue, close_request);
    uv_async_send(&sock->async_handle);

    return COBRA_SOCKET_OK;
}

void cobra__socket_close(cobra_socket_t *sock, int error) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->connection_status == COBRA_SOCKET_STATUS_CLOSING) {
        uv_mutex_unlock(&sock->mutex_handle);
        return;
    }

    sock->connection_status = COBRA_SOCKET_STATUS_CLOSING;
    uv_close((uv_handle_t *) &sock->tcp_handle,
             cobra__socket_close_callback);

    uv_mutex_unlock(&sock->mutex_handle);
}

void cobra__socket_close_callback(uv_handle_t *tcp_handle) {
    cobra_socket_t *sock = tcp_handle->data;

    // Necessary to re-init handles after uv_close
    uv_tcp_init(&sock->loop, &sock->tcp_handle);

    sock->connection_status = COBRA_SOCKET_STATUS_CLOSED;
    sock->connection_alive = false;
    cobra_buffer_clear(&sock->read_buffer);
    sock->read_packet_body_length = 0;
    sock->write_queue_length = 0;

    if (sock->close_callback) {
        sock->close_callback(socket, socket->close_error);
    }

}