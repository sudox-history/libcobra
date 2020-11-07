#include <stdlib.h>

#define COBRA_TCP_CONNECTION_PRIVATE
#include "tcp_connection.h"

cobra_tcp_connection_t *cobra_tcp_connection_create(
        connect_cb on_connect,
        close_cb on_close,
        data_cb on_data,
        activity_cb on_activity) {
    cobra_tcp_connection_t *connection = malloc(sizeof(cobra_tcp_connection_t));

    uv_loop_init(&connection->loop);
    uv_tcp_init(&connection->loop, &connection->tcp_handle);

    connection->loop.data = connection;
    connection->tcp_handle.data = connection;
    connection->getaddrinfo_req.data = connection;
    connection->connect_req.data = connection;
    connection->write_req.data = connection;

    connection->connected = false;
    connection->buffer = cobra_buffer_create(COBRA_TCP_CONNECTION_MAX_PACKET_SIZE);
    connection->packet_len = 0;

    connection->on_connect = on_connect;
    connection->on_close = on_close;
    connection->on_data = on_data;
    connection->on_activity = on_activity;

    return connection;
}

void cobra_tcp_connection_destroy(cobra_tcp_connection_t *connection) {
    if (connection->connected)
        cobra_tcp_connection_close(connection);

    cobra_buffer_destroy(connection->buffer);
    free(connection);
}

void on_alloc(uv_handle_t *handle, size_t _, uv_buf_t *buf) {
    cobra_tcp_connection_t *connection = handle->data;

    uint8_t *base;
    int len = cobra_buffer_write_pointer(connection->buffer, &base);

    buf->base = (char *) base;
    buf->len = len;
}

void on_read(uv_stream_t *stream, ssize_t read_len, const uv_buf_t *buf) {
    cobra_tcp_connection_t *connection = stream->data;

    if (read_len <= 0) {
        if (connection->on_close)
            connection->on_close(connection, COBRA_TCP_CONNECTION_OK);

        return;
    }

    if (connection->on_activity)
        connection->on_activity(connection);

    // Skipping wrote bytes
    cobra_buffer_skip(connection->buffer, read_len);

    while (true) {
        // We don't have packet_header
        if (!connection->packet_len) {
            // Packet header still not received
            if (cobra_buffer_len(connection->buffer) < sizeof(uint16_t))
                break;

            // Reading header
            connection->packet_len = cobra_buffer_read_uint16(connection->buffer);
            if (!connection->packet_len)
                continue;
        }

        // Packet not fully received
        if (connection->packet_len > cobra_buffer_len(connection->buffer))
            break;

        uint8_t *packet_body = malloc(connection->packet_len);
        cobra_buffer_read(connection->buffer, packet_body, connection->packet_len);

        if (connection->on_data)
            connection->on_data(connection, packet_body, connection->packet_len);

        connection->packet_len = 0;
    }

    cobra_buffer_fragment(connection->buffer);
}

void on_connected(uv_connect_t *connect_req, int status) {
    cobra_tcp_connection_t *connection = connect_req->data;

    if (status) {
        if (connection->on_close)
            connection->on_close(connection, COBRA_TCP_CONNECTION_ERR_CONNECTING);

        return;
    }

    uv_read_start((uv_stream_t *) &connection->tcp_handle, on_alloc, on_read);

    if (connection->on_connect)
        connection->on_connect(connection);
}

void on_close(uv_handle_t *handle) {
    cobra_tcp_connection_t *connection = handle->data;

    if (connection->on_close)
        connection->on_close(connection, COBRA_TCP_CONNECTION_OK);
}

void on_resolved(uv_getaddrinfo_t *getaddrinfo_req, int status, struct addrinfo *ai) {
    cobra_tcp_connection_t *connection = getaddrinfo_req->data;

    if (status) {
        if (connection->on_close)
            connection->on_close(connection, COBRA_TCP_CONNECTION_ERR_RESOLUTION);

        return;
    }

    uv_tcp_connect(&connection->connect_req,
                   &connection->tcp_handle,
                   ai->ai_addr,
                   on_connected);

    // Specified in libuv documentation
    uv_freeaddrinfo(ai);
}

int cobra_tcp_connection_connect(cobra_tcp_connection_t *connection, char *host, char *port) {
    if (connection->connected)
        return COBRA_TCP_CONNECTION_ERR_ALREADY_CONNECTED;

    connection->connected = true;

    uv_getaddrinfo(&connection->loop,
                   &connection->getaddrinfo_req,
                   on_resolved,
                   host,
                   port,
                   NULL);

    uv_run(&connection->loop, UV_RUN_DEFAULT);
    return COBRA_TCP_CONNECTION_OK;
}

int cobra_tcp_connection_close(cobra_tcp_connection_t *connection) {
    if (!connection->connected)
        return COBRA_TCP_CONNECTION_ERR_NOT_CONNECTED;

    connection->connected = false;

    uv_close((uv_handle_t *) &connection->tcp_handle, on_close);
    return COBRA_TCP_CONNECTION_OK;
}

bool is_platform_big_endian() {
    int number = 1;
    return *(char *) &number == 0;
}

int cobra_tcp_connection_send(cobra_tcp_connection_t *connection, uint8_t *data, uint16_t len) {
    if (!connection->connected)
        return COBRA_TCP_CONNECTION_ERR_NOT_CONNECTED;

    // TODO: Rewrite this method

    int full_packet_len = len + (int) sizeof(uint16_t);

    uint8_t packet[full_packet_len];
    memcpy(packet + sizeof(uint16_t), data, len);

    uint16_t packet_len = len;

    if (!is_platform_big_endian())
        packet_len = packet_len >> 8 | packet_len << 8; // NOLINT(hicpp-signed-bitwise)

    memcpy(packet, (uint8_t *) &packet_len, sizeof(uint16_t));

    uv_buf_t write_buffer = {
            .base = (char *) packet,
            .len = full_packet_len
    };

    uv_write(&connection->write_req,
             (uv_stream_t *) &connection->tcp_handle,
             &write_buffer,
             1,
             NULL);

    return COBRA_TCP_CONNECTION_OK;
}