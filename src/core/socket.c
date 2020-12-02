#define COBRA_SOCKET_PRIVATE
#include "socket.h"

cobra_socket_t *cobra_socket_create(int write_queue_size) {
    cobra_socket_t *socket = malloc(sizeof(cobra_socket_t));

    uv_loop_init(&socket->loop);
    uv_tcp_init(&socket->loop, &socket->tcp_handle);
    socket->loop.data = socket;
    socket->tcp_handle.data = socket;

    socket->is_connected = false;
    socket->is_alive = false;
    socket->is_write_queue_full = false;
    socket->is_closing = false;
    socket->close_error = COBRA_SOCKET_OK;

    socket->write_queue_size = write_queue_size;
    socket->write_queue_length = 0;

    cobra_buffer_init(&socket->read_buffer, COBRA_SOCKET_PACKET_MAX_LENGTH);
    socket->read_packet_body_length = 0;

    socket->on_connect = NULL;
    socket->on_close = NULL;
    socket->on_alloc = NULL;
    socket->on_data = NULL;
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

static void cobra__socket_on_close(uv_handle_t *handle) {
    cobra_socket_t *socket = handle->data;

    // Necessary to re-init handles after uv_close
    uv_tcp_init(&socket->loop, &socket->tcp_handle);

    socket->is_connected = false;
    socket->is_alive = false;
    socket->is_write_queue_full = false;
    socket->is_closing = false;

    socket->write_queue_length = 0;

    cobra_buffer_clear(&socket->read_buffer);
    socket->read_packet_body_length = 0;

    if (socket->on_close)
        socket->on_close(socket, socket->close_error);
}

void cobra__socket_close(cobra_socket_t *socket, int error) {
    if (socket->is_closing)
        return;

    socket->is_closing = true;
    socket->close_error = error;

    uv_close((uv_handle_t *) &socket->tcp_handle,
             cobra__socket_on_close);
}

void cobra__socket_on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *read_buffer) {
    cobra_socket_t *socket = handle->data;

    read_buffer->base = (char *) cobra_buffer_write_pointer(&socket->read_buffer);
    read_buffer->len = cobra_buffer_capacity(&socket->read_buffer);
}

void cobra__socket_on_data(uv_stream_t *stream_handle, ssize_t read_length, const uv_buf_t *buf) {
    cobra_socket_t *socket = stream_handle->data;

    if (read_length <= 0) {
        cobra__socket_close(socket, COBRA_SOCKET_OK);
        return;
    }

    // TODO: Update timer

    // Skipping wrote bytes
    cobra_buffer_write_void(&socket->read_buffer, read_length);

    while (true) {
        if (!socket->read_packet_body_length) {
            // Packet header not received
            if (cobra_buffer_length(&socket->read_buffer) < COBRA_SOCKET_PACKET_HEADER_LENGTH)
                break;

            // Reading header
            socket->read_packet_body_length =
                    cobra_buffer_read_uint(&socket->read_buffer, COBRA_SOCKET_PACKET_HEADER_LENGTH);

            // If packet length == 0 we received ping packet
            if (socket->read_packet_body_length == 0)
                // TODO: Handle ping packet
                continue;
        }

        // Packet body not received
        if (socket->read_packet_body_length > cobra_buffer_length(&socket->read_buffer))
            break;

        // Handling packet if we have on_alloc & on_data callbacks
        if (socket->on_alloc && socket->on_data) {
            uint8_t *packet_body;
            socket->on_alloc(socket, &packet_body, socket->read_packet_body_length);

            cobra_buffer_read(&socket->read_buffer, packet_body, socket->read_packet_body_length);
            socket->on_data(socket, packet_body, socket->read_packet_body_length);
        } else
            cobra_buffer_read_void(&socket->read_buffer, socket->read_packet_body_length);

        socket->read_packet_body_length = 0;
    }

    cobra_buffer_fragment(&socket->read_buffer);
}

void cobra__socket_on_connect(uv_connect_t *connect_req, int error) {
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

void cobra__socket_on_resolve(uv_getaddrinfo_t *getaddrinfo_req, int error, struct addrinfo *addrinfo) {
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
                   cobra__socket_on_connect);

    // Specified in libuv docs
    uv_freeaddrinfo(addrinfo);
    free(getaddrinfo_req);
}

int cobra_socket_connect(cobra_socket_t *socket, char *host, char *port) {
    if (socket->is_connected)
        return COBRA_SOCKET_ERR_ALREADY_CONNECTED;

    socket->host = host;
    socket->port = port;
    socket->is_connected = true;
    socket->is_alive = true;

    uv_getaddrinfo_t *getaddrinfo_req = malloc(sizeof(uv_getaddrinfo_t));
    getaddrinfo_req->data = socket;

    uv_getaddrinfo(&socket->loop,
                   getaddrinfo_req,
                   cobra__socket_on_resolve,
                   host,
                   port,
                   NULL);

    uv_run(&socket->loop, UV_RUN_DEFAULT);
    return COBRA_SOCKET_OK;
}

int cobra_socket_close(cobra_socket_t *socket) {
    if (!socket->is_connected)
        return COBRA_SOCKET_ERR_NOT_CONNECTED;

    cobra__socket_close(socket, COBRA_SOCKET_OK);
    return COBRA_SOCKET_OK;
}

typedef struct cobra__socket_write_context {
    cobra_socket_t *socket;
    cobra_buffer_t packet;
    uv_write_t write_req;
} cobra__socket_write_context;

void cobra__socket_on_write(uv_write_t *write_req, int error) {
    cobra__socket_write_context *context = write_req->data;

    if (error) {
        cobra__socket_close(context->socket, COBRA_SOCKET_ERR_WRITING);
        return;
    }

    // Calling drain callback only if we need it
    if (context->socket->on_drain && context->socket->is_write_queue_full) {
        context->socket->on_drain(context->socket);
        context->socket->is_write_queue_full = false;
    }

    // Decreasing queue length
    context->socket->write_queue_length -= 1;

    cobra_buffer_deinit(&context->packet);
    free(context);
}

int cobra_socket_send(cobra_socket_t *socket, uint8_t *data, uint64_t length) {
    if (!socket->is_connected)
        return COBRA_SOCKET_ERR_NOT_CONNECTED;

    if (socket->write_queue_length == socket->write_queue_size)
        return COBRA_SOCKET_ERR_QUEUE_OVERFLOW;

    cobra__socket_write_context *context = malloc(sizeof(cobra__socket_write_context));
    context->socket = socket;
    cobra_buffer_init(&context->packet, length + COBRA_SOCKET_PACKET_HEADER_LENGTH);
    context->write_req.data = context;

    cobra_buffer_write_uint(&context->packet,
                            length,
                            COBRA_SOCKET_PACKET_HEADER_LENGTH);
    cobra_buffer_write(&context->packet, data, length);

    uv_buf_t write_buffer = {
            .base = (char *) cobra_buffer_read_pointer(&context->packet),
            .len = cobra_buffer_length(&context->packet)
    };

    uv_write(&context->write_req,
             (uv_stream_t *) &socket->tcp_handle,
             &write_buffer,
             1,
             cobra__socket_on_write);

    // Increasing queue length
    context->socket->write_queue_length += 1;

    if (socket->write_queue_length == socket->write_queue_size) {
        socket->is_write_queue_full = true;
        return COBRA_SOCKET_ERR_QUEUE_FULL;
    }

    return COBRA_SOCKET_OK;
}

void cobra_socket_set_data(cobra_socket_t *socket, void *data) {
    socket->data = data;
}

void *cobra_socket_get_data(cobra_socket_t *socket) {
    return socket->data;
}

void cobra_socket_set_callbacks(cobra_socket_t *socket, cobra_socket_connect_cb on_connect,
                                cobra_socket_close_cb on_close,
                                cobra_socket_alloc_cb on_alloc,
                                cobra_socket_data_cb on_data,
                                cobra_socket_drain_cb on_drain) {
    socket->on_connect = on_connect;
    socket->on_close = on_close;
    socket->on_alloc = on_alloc;
    socket->on_data = on_data;
    socket->on_drain = on_drain;
}