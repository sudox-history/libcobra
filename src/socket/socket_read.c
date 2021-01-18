#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

void cobra__socket_start_read(cobra_socket_t *sock) {
    uv_read_start((uv_stream_t *)&sock->tcp_handle,
                  cobra__socket_alloc_callback, cobra__socket_read_callback);
}

void cobra__socket_alloc_callback(uv_handle_t *tcp_handle,
                                  size_t length,
                                  uv_buf_t *read_buffer) {
    cobra_socket_t *sock = tcp_handle->data;

    read_buffer->base = (char *)cobra_buffer_write_pointer(&sock->read_buffer);
    read_buffer->len = cobra_buffer_capacity(&sock->read_buffer);
}

void cobra__socket_read_callback(uv_stream_t *tcp_handle,
                                 ssize_t length,
                                 const uv_buf_t *read_buffer) {
    cobra_socket_t *sock = tcp_handle->data;

    if (length <= 0) {
        cobra__socket_close(sock, COBRA_SOCKET_OK);
        return;
    }

    cobra_buffer_write_void(&sock->read_buffer, length);
    cobra__socket_ping_refresh_timer(sock);

    while (true) {
        if (!sock->read_frame_body_length) {
            // Packet header not received
            if (cobra_buffer_length(&sock->read_buffer) <
                COBRA_SOCKET_FRAME_HEADER_LENGTH)
                break;

            // Reading header
            sock->read_frame_body_length = cobra_buffer_read_uint(
                &sock->read_buffer, COBRA_SOCKET_FRAME_HEADER_LENGTH);

            // If packet body length == 0 we received ping packet
            if (sock->read_frame_body_length == 0) {
                cobra__socket_ping_frame(sock);
                continue;
            }
        }

        // Packet body not received
        if (sock->read_frame_body_length >
            cobra_buffer_length(&sock->read_buffer))
            break;

        uv_mutex_lock(&sock->mutex_handle);
        cobra_socket_alloc_cb alloc_callback = sock->alloc_callback;
        cobra_socket_read_cb read_callback = sock->read_callback;
        uv_mutex_unlock(&sock->mutex_handle);

        // Handling packet if we have alloc_callback & read_callback
        if (alloc_callback != NULL && read_callback != NULL) {
            uint8_t *packet_body;
            alloc_callback(sock, &packet_body, sock->read_frame_body_length);

            cobra_buffer_read(&sock->read_buffer, packet_body,
                              sock->read_frame_body_length);
            read_callback(sock, packet_body, sock->read_frame_body_length);
        } else
            cobra_buffer_read_void(&sock->read_buffer,
                                   sock->read_frame_body_length);

        sock->read_frame_body_length = 0;
    }

    cobra_buffer_fragment(&sock->read_buffer);
}