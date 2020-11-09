#define COBRA_TCP_SERVER_PRIVATE
#include "tcp_server.h"

cobra_tcp_server_t *cobra_server_create() {
    cobra_tcp_server_t *server = malloc(sizeof(cobra_tcp_server_t));

    uv_loop_init(&server->loop);
    uv_tcp_init(&server->loop, &server->tcp_handle);

    server->loop.data = server;
    server->tcp_handle.data = server;

    return server;
}

void cobra_tcp_server_destroy(cobra_tcp_server_t *server) {
    free(server);
}

void cobra_tcp_server_set_callbacks(cobra_tcp_server_t *server,
                                    server_connection_cb on_connection,
                                    server_close_cb on_close) {
    server->on_connection = on_connection;
    server->on_close = on_close;
}

void server_on_connection(uv_stream_t *stream, int status) {
    cobra_tcp_server_t *server = stream->data;

    if (status) {
        if (server->on_close)
            server->on_close(server, COBRA_TCP_SERVER_ERR_LISTENING);

        return;
    }

    cobra_tcp_connection_t *connection = cobra_tcp_connection_create();
    connection->loop = *stream->loop;
    connection->connected = true;

    uv_tcp_init(&server->loop, &connection->tcp_handle);
    uv_accept(stream, (uv_stream_t *) &connection->tcp_handle);

    if (server->on_connection)
        server->on_connection(server, connection);

    uv_read_start((uv_stream_t *) &connection->tcp_handle,
                  on_alloc,
                  on_read);
}

int cobra_tcp_server_listen(cobra_tcp_server_t *server, char *host, int port) {
    if (server->listening)
        return COBRA_TCP_SERVER_ERR_ALREADY_LISTENING;

    server->listening = true;

    struct sockaddr_in addr;
    uv_ip4_addr(host, port, &addr);

    if (uv_tcp_bind(&server->tcp_handle,
                    (const struct sockaddr *) &addr,
                    0))
        return COBRA_TCP_SERVER_ERR_BINDING;

    if (uv_listen((uv_stream_t *) &server->tcp_handle,
                  128,
                  server_on_connection))
        return COBRA_TCP_SERVER_ERR_LISTENING;

    uv_run(&server->loop, UV_RUN_DEFAULT);
    return COBRA_TCP_SERVER_OK;
}

void server_on_close(uv_handle_t *handle) {
    cobra_tcp_server_t *server = handle->data;

    if (server->on_close)
        server->on_close(server, COBRA_TCP_SERVER_OK);
}

int cobra_tcp_server_close(cobra_tcp_server_t *server) {
    if (!server->listening)
        return COBRA_TCP_SERVER_ERR_NOT_LISTENING;

    server->listening = false;

    uv_close((uv_handle_t *) &server->tcp_handle, server_on_close);
    return COBRA_TCP_CONNECTION_OK;
}

void cobra_tcp_server_set_data(cobra_tcp_server_t *server, void *data) {
    server->data = data;
}

void *cobra_tcp_server_get_data(cobra_tcp_server_t *server) {
    return server->data;
}