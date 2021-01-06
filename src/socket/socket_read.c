#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

void cobra__socket_alloc_callback(uv_handle_t *tcp_handle,
                                  size_t length,
                                  uv_buf_t *read_buffer) {
    cobra_socket_t *sock = tcp_handle->data;

    read_buffer->base = (char *) cobra_buffer_write_pointer(&sock->read_buffer);
    read_buffer->len = cobra_buffer_capacity(&sock->read_buffer);
}

void cobra__socket_read_callback(uv_stream_t *tcp_handle,
                                 ssize_t length,
                                 const uv_buf_t *read_buffer) {
    cobra_socket_t *sock = tcp_handle->data;

    if (length <= 0) {
        // TODO: Close connection with OK status
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);

    // Skipping wrote bytes
    cobra_buffer_write_void(&sock->read_buffer, length);

    while (true) {
        if (!sock->read_packet_body_length) {
            // Packet header not received
            if (cobra_buffer_length(&sock->read_buffer) < COBRA_SOCKET_PACKET_HEADER_LENGTH) {
                break;
            }

            // Reading header
            sock->read_packet_body_length =
                    cobra_buffer_read_uint(&sock->read_buffer, COBRA_SOCKET_PACKET_HEADER_LENGTH);

            // If packet body length == 0 we received ping packet
            if (sock->read_packet_body_length == 0) {
                // TODO: Handle ping packet
                continue;
            }
        }

        // Packet body not received
        if (sock->read_packet_body_length > cobra_buffer_length(&sock->read_buffer)) {
            break;
        }

        // Handling packet if we have alloc_callback & read_callback
        if (sock->alloc_callback && sock->read_callback) {
            uint8_t *packet_body;
            sock->alloc_callback(sock, &packet_body, sock->read_packet_body_length);

            cobra_buffer_read(&sock->read_buffer, packet_body, sock->read_packet_body_length);
            sock->read_callback(sock, packet_body, sock->read_packet_body_length);
        }
        else {
            cobra_buffer_read_void(&sock->read_buffer, sock->read_packet_body_length);
        }

        sock->read_packet_body_length = 0;
    }

    cobra_buffer_fragment(&sock->read_buffer);
    uv_mutex_unlock(&sock->mutex_handle);
}