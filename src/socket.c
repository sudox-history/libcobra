#define COBRA_SOCKET_PRIVATE
#include "socket.h"

cobra_socket_t *cobra_socket_create(int write_queue_size) {
    cobra_socket_t *socket = malloc(sizeof(cobra_socket_t));

    uv_loop_init(&socket->loop);
    uv_tcp_init(&socket->loop, &socket->tcp_handle);
    socket->loop.data = socket;
    socket->tcp_handle.data = socket;

    socket->connected = false;
    socket->write_queue_size = write_queue_size;
    socket->write_queue_length = 0;
    socket->read_buffer = cobra_buffer_create(COBRA_SOCKET_PACKET_MAX_SIZE);
    socket->read_packet_body_length = 0;

    socket->on_connect = NULL;
    socket->on_close = NULL;
    socket->on_alloc = NULL;
    socket->on_data = NULL;

    return socket;
}

void cobra_socket_destroy(cobra_socket_t *socket) {
    if (socket->connected)
        cobra_socket_close(socket);

    cobra_buffer_destroy(socket->read_buffer);
    free(socket);
}

static void cobra__socket_on_close(uv_handle_t *handle) {
    cobra_socket_t *socket = handle->data;

    socket->connected = false;
    if (socket->on_close)
        socket->on_close(socket, socket->close_error);
}

static void cobra__socket_close(cobra_socket_t *socket, int error) {
    socket->close_error = error;

    uv_close((uv_handle_t *) &socket->tcp_handle,
             cobra__socket_on_close);
}

static void cobra__socket_on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *buf) {
    cobra_socket_t *socket = handle->data;

    buf->base = (char *) cobra_buffer_write_pointer(socket->read_buffer);
    buf->len = cobra_buffer_capacity(socket->read_buffer);
}

static void cobra__socket_on_data(uv_stream_t *stream_handle, ssize_t read_length, const uv_buf_t *buf) {
    cobra_socket_t *socket = stream_handle->data;

    if (read_length <= 0) {
        cobra__socket_close(socket, COBRA_SOCKET_OK);
        return;
    }

    // TODO: Update timer

    // Skipping wrote bytes
    cobra_buffer_write_void(socket->read_buffer, read_length);

    while (true) {
        if (!socket->read_packet_body_length) {
            // Packet header not received
            if (cobra_buffer_length(socket->read_buffer) < COBRA_SOCKET_PACKET_HEADER_LENGTH)
                break;

            // Reading header
            socket->read_packet_body_length = cobra_buffer_read_uint(
                    socket->read_buffer,
                    COBRA_SOCKET_PACKET_HEADER_LENGTH
            );

            // Packet length includes header length
            socket->read_packet_body_length -= COBRA_SOCKET_PACKET_HEADER_LENGTH;

            // If packet length == 0 we received ping packet
            if (socket->read_packet_body_length == 0)
                // TODO: Handle ping packet
                continue;
        }

        // Packet body not received
        if (socket->read_packet_body_length > cobra_buffer_length(socket->read_buffer))
            break;

        // Handling packet if we have on_alloc & on_data callbacks
        if (socket->on_alloc && socket->on_data) {
            uint8_t *packet_body;
            socket->on_alloc(socket, &packet_body, socket->read_packet_body_length);

            cobra_buffer_read(socket->read_buffer, packet_body, socket->read_packet_body_length);
            socket->on_data(socket, packet_body, socket->read_packet_body_length);
        } else
            cobra_buffer_read_void(socket->read_buffer, socket->read_packet_body_length);

        socket->read_packet_body_length = 0;
    }

    cobra_buffer_fragment(socket->read_buffer);
}

static void cobra__socket_on_connected(uv_connect_t *connect_req, int error) {
    cobra_socket_t *socket = connect_req->data;

    if (error) {
        cobra__socket_close(socket, COBRA_SOCKET_ERR_CONNECTING);
        return;
    }

    if (socket->on_connect)
        socket->on_connect(socket);

    uv_read_start((uv_stream_t *) &socket->tcp_handle,
                  cobra__socket_on_alloc,
                  cobra__socket_on_data);

    free(connect_req);
}

static void cobra__socket_on_resolved(uv_getaddrinfo_t *getaddrinfo_req, int error, struct addrinfo *addrinfo) {
    cobra_socket_t *socket = getaddrinfo_req->data;

    if (error) {
        cobra__socket_close(socket, COBRA_SOCKET_ERR_RESOLVING);
        return;
    }

    uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));
    connect_req->data = socket;

    uv_tcp_connect(connect_req,
                   &socket->tcp_handle,
                   addrinfo->ai_addr,
                   cobra__socket_on_connected);

    // Specified in libuv docs
    uv_freeaddrinfo(addrinfo);
    free(getaddrinfo_req);
}

int cobra_socket_connect(cobra_socket_t *socket, char *host, char *port) {
    if (socket->connected)
        return COBRA_SOCKET_ERR_ALREADY_CONNECTED;

    socket->connected = true;
    socket->host = host;
    socket->port = port;

    uv_getaddrinfo_t *getaddrinfo_req = malloc(sizeof(uv_getaddrinfo_t));
    getaddrinfo_req->data = socket;

    uv_getaddrinfo(&socket->loop,
                   getaddrinfo_req,
                   cobra__socket_on_resolved,
                   host,
                   port,
                   NULL);

    uv_run(&socket->loop, UV_RUN_DEFAULT);
    return COBRA_SOCKET_OK;
}

int cobra_socket_close(cobra_socket_t *socket) {
    if (!socket->connected)
        return COBRA_SOCKET_ERR_NOT_CONNECTED;

    cobra__socket_close(socket, COBRA_SOCKET_OK);
    return COBRA_SOCKET_OK;
}

int cobra_socket_send(cobra_socket_t *socket, uint8_t *data, uint64_t length) {

}