#include <stdlib.h>
#include <uv.h>

#define COBRA_TRANSPORT_CLIENT_PRIVATE

#include "transport_connection.h"
#include "transport_constants.h"

transport_connection_t *transport_connection_create() {
    transport_connection_t *connection = malloc(sizeof(transport_connection_t));

    // Initializing libuv loop
    uv_loop_init(&connection->loop);
    connection->loop.data = connection;

    // Initializing libuv tcp handle
    uv_tcp_init(&connection->loop, &connection->tcp_handle);
    connection->tcp_handle.data = connection;

    // Initializing libuv tcp connect req
    connection->tcp_connect_req.data = connection;

    return connection;
}

void transport_connection_destroy(transport_connection_t *client) {
    free(client);
}

void on_read(uv_stream_t *stream, ssize_t len, const uv_buf_t *buf) {
    transport_connection_t *connection = stream->data;

    for (int i = 0; i < len; i++) {
        printf("%d ", buf->base[i]);
    }
}

void on_alloc(uv_handle_t *handle, size_t suggested_len, uv_buf_t *buf) {
    buf->base = malloc(65535);
    buf->len = 65535;
}

void on_connected(uv_connect_t *connect_req, int status) {
    transport_connection_t *connection = connect_req->data;

    printf("%s\n", uv_strerror(status));

    if (status) {
        if (connection->on_error) {
            connection->on_error(connection, COBRA_TRANSPORT_CONNECTION_ERR);
        }

        return;
    }

    if (connection->on_connect) {
        connection->on_connect(connection);
    }

    printf("Connected to server\n");

    uv_read_start((uv_stream_t *) &connection->tcp_handle, on_alloc, on_read);
}

void on_resolved(uv_getaddrinfo_t *getaddr_req, int status, struct addrinfo *ai) {
    transport_connection_t *connection = getaddr_req->data;

    if (status) {
        if (connection->on_error) {
            connection->on_error(connection, COBRA_TRANSPORT_CONNECTION_ERR);
        }

        return;
    }

    // Trying to connect
    uv_tcp_connect(&connection->tcp_connect_req,
                   &connection->tcp_handle,
                   ai->ai_addr,
                   on_connected);

    uv_freeaddrinfo(ai);
}

void transport_connection_connect(transport_connection_t *connection, char *host, char *port) {
    uv_getaddrinfo_t getaddr_req = {
            .data = connection
    };

    // In async mode function always returns 0
    uv_getaddrinfo(&connection->loop,
                   &getaddr_req,
                   on_resolved,
                   host,
                   port,
                   NULL);

    uv_run(&connection->loop, UV_RUN_DEFAULT);
}