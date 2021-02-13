#define COBRA_SERVER_PRIVATE
#include "cobra/server.h"

cobra_server_err_t cobra_server_listen(cobra_server_t *server,
                                       char *host,
                                       char *port) {
    uv_mutex_lock(&server->mutex_handle);

    if (server->state != COBRA_SERVER_STATE_CLOSED) {
        uv_mutex_unlock(&server->mutex_handle);
        return COBRA_SERVER_ERR_ALREADY_OPENED;
    }

    server->state = COBRA_SERVER_STATE_INITIALIZING;

    // Necessary to re-init handles each time you want to open connection.
    // Initialization MUST be under mutex protection because after unlock()
    // user can call immediately cobra_server_close() which deals with loop.
    uv_tcp_init(&server->loop, &server->tcp_handle);
    cobra_async_bind(&server->close_async, &server->loop);
    uv_mutex_unlock(&server->mutex_handle);

    cobra__server_listen_ctx_t *listen_ctx =
        malloc(sizeof(cobra__server_listen_ctx_t));

    char *host_copy = malloc(strlen(host) + 1);
    char *port_copy = malloc(strlen(port) + 1);

    strcpy(host_copy, host);
    strcpy(port_copy, port);

    listen_ctx->server = server;
    listen_ctx->host = host_copy;
    listen_ctx->port = port_copy;

    uv_thread_create(&server->thread_handle, cobra_server_listen_thread,
                     listen_ctx);
    return COBRA_SERVER_OK;
}

void cobra_server_listen_thread(void *data) {
    cobra__server_listen_ctx_t *listen_ctx = data;

    cobra_server_t *server = listen_ctx->server;
    char *host = listen_ctx->host;
    char *port = listen_ctx->port;

    // Don't forget to free context
    free(listen_ctx);

    uv_mutex_lock(&server->mutex_handle);

    // During thread creation user could run cobra_socket_close()
    if (server->state != COBRA_SERVER_STATE_CLOSING) {
        server->state = COBRA_SERVER_STATE_RESOLVING;
        uv_mutex_unlock(&server->mutex_handle);

        server->resolve_request = malloc(sizeof(uv_getaddrinfo_t));
        uv_req_set_data((uv_req_t *)server->resolve_request, server);

        uv_getaddrinfo(&server->loop, server->resolve_request,
                       cobra__server_resolve_callback, host, port, NULL);

        free(host);
        free(port);
    } else {
        uv_mutex_unlock(&server->mutex_handle);
    }

    uv_run(&server->loop, UV_RUN_DEFAULT);
}

void cobra__server_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo) {
    cobra_server_t *server = uv_req_get_data((uv_req_t *)resolve_request);

    free(server->resolve_request);
    server->resolve_request = NULL;

    // If user called cobra_server_socket() during connecting
    if (error == UV_ECANCELED)
        return;

    if (error != 0) {
        cobra__server_close(server, COBRA_SERVER_ERR_RESOLVING);
        return;
    }

    if (uv_tcp_bind(&server->tcp_handle, addrinfo->ai_addr, 0) != 0) {
        cobra__server_close(server, COBRA_SERVER_ERR_BINDING);
        return;
    }

    if (uv_listen((uv_stream_t *)&server->tcp_handle, COBRA_SERVER_BACKLOG,
                  cobra__server_connection_callback) != 0) {
        cobra__server_close(server, COBRA_SERVER_ERR_LISTENING);
        return;
    }

    uv_freeaddrinfo(addrinfo);
}

void cobra__server_connection_callback(uv_stream_t *tcp_handle, int error) {
    cobra_server_t *server = tcp_handle->data;

    if (error != 0) {
        cobra__server_close(server, COBRA_SERVER_OK);
        return;
    }

    cobra_socket_t *sock =
        cobra_socket_create(server->sockets_write_queue_size);
    cobra__socket_bind(sock, &server->loop);

    uv_accept((uv_stream_t *)&server->tcp_handle,
              (uv_stream_t *)cobra__socket_get_tcp_handle(sock));

    cobra__socket_start_read(sock);
    cobra__socket_start_ping(sock);

    uv_mutex_lock(&server->mutex_handle);
    cobra_server_connection_cb connection_callback =
        server->connection_callback;
    uv_mutex_unlock(&server->mutex_handle);

    // TODO: Potential memory leak (socket isn't freed if there is no
    // connection_callback)
    if (connection_callback)
        connection_callback(server, sock);
}
