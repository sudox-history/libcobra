#define COBRA_SERVER_PRIVATE
#include "server_old.h"

cobra_server_t *cobra_server_create(int socket_write_queue_size) {
    cobra_server_t *server = malloc(sizeof(cobra_server_t));

    uv_loop_init(&server->loop);
    uv_tcp_init(&server->loop, &server->tcp_handle);
    server->loop.data = server;
    server->tcp_handle.data = server;
    server->socket_write_queue_size = socket_write_queue_size;

    server->is_listening = false;
    server->is_closing = false;
    server->close_error = COBRA_SERVER_OK;

    server->on_connection = NULL;
    server->on_close = NULL;

    server->data = NULL;
    return server;
}

void cobra_server_destroy(cobra_server_t *server) {
    free(server);
}

void cobra__server_on_close(uv_handle_t *handle) {
    cobra_server_t *server = handle->data;

    // Necessary to re-init handles after uv_close
    uv_tcp_init(&server->loop, &server->tcp_handle);

    server->is_listening = false;
    server->is_closing = false;

    if (server->on_close)
        server->on_close(server, server->close_error);
}

void cobra__server_close(cobra_server_t *server, int error) {
    if (server->is_closing)
        return;

    server->is_closing = true;
    server->close_error = error;

    uv_close((uv_handle_t *) &server->tcp_handle,
             cobra__server_on_close);
}

void cobra__server_on_connection(uv_stream_t *stream_handle, int error) {
    cobra_server_t *server = stream_handle->data;

    if (error) {
        cobra__server_close(server, COBRA_SERVER_ERR_LISTENING);
        return;
    }

    cobra_socket_t *socket = cobra_socket_create(server->socket_write_queue_size);

    // Re-init socket's handles to server's loop
    uv_tcp_init(&server->loop, &socket->tcp_handle);
    // TODO: Ping

    // Filling necessary fields
    // TODO: Fill host & port
    socket->is_connected = true;
    socket->is_alive = true;

    uv_accept((uv_stream_t *) &server->tcp_handle,
              (uv_stream_t *) &socket->tcp_handle);

    if (server->on_connection)
        server->on_connection(server, socket);

    // Starting reading
    uv_read_start((uv_stream_t *) &socket->tcp_handle,
                  cobra__socket_on_alloc,
                  cobra__socket_on_data);
}

void cobra__server_on_resolve(uv_getaddrinfo_t *getaddrinfo_req, int error, struct addrinfo *addrinfo) {
    cobra_server_t *server = getaddrinfo_req->data;

    if (error) {
        cobra__server_close(server, COBRA_SERVER_ERR_RESOLVING);
        return;
    }

    if (uv_tcp_bind(&server->tcp_handle,
                    addrinfo->ai_addr,
                    0)) {
        cobra__server_close(server, COBRA_SERVER_ERR_BINDING);
        return;
    }

    if (uv_listen((uv_stream_t *) &server->tcp_handle,
                  COBRA_SERVER_BACKLOG,
                  cobra__server_on_connection)) {
        cobra__server_close(server, COBRA_SERVER_ERR_LISTENING);
        return;
    }

    // Specified in libuv docs
    uv_freeaddrinfo(addrinfo);
    free(getaddrinfo_req);
}

int cobra_server_listen(cobra_server_t *server, char *host, char *port) {
    if (server->is_listening)
        return COBRA_SERVER_ERR_ALREADY_LISTENING;

    server->host = host;
    server->port = port;
    server->is_listening = true;

    uv_getaddrinfo_t *getaddrinfo_req = malloc(sizeof(uv_getaddrinfo_t));
    getaddrinfo_req->data = server;

    uv_getaddrinfo(&server->loop,
                   getaddrinfo_req,
                   cobra__server_on_resolve,
                   host,
                   port,
                   NULL);

    uv_run(&server->loop, UV_RUN_DEFAULT);
    return COBRA_SERVER_OK;
}

int cobra_server_close(cobra_server_t *server) {
    if (!server->is_listening)
        return COBRA_SERVER_ERR_ALREADY_LISTENING;

    cobra__server_close(server, COBRA_SERVER_OK);
    return COBRA_SERVER_OK;
}

void cobra_server_set_data(cobra_server_t *server, void *data) {
    server->data = data;
}

void *cobra_server_get_data(cobra_server_t *server) {
    return server->data;
}

void cobra_server_set_callbacks(cobra_server_t *server,
                                cobra_server_connection_cb on_connection,
                                cobra_server_close_cb on_close) {
    server->on_connection = on_connection;
    server->on_close = on_close;
}