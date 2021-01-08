#define COBRA_SOCKET_PRIVATE
#include "cobra/socket.h"

int cobra_socket_connect(cobra_socket_t *sock, char *host, char *port) {
    uv_mutex_lock(&sock->mutex_handle);

    if (sock->connection_status != COBRA_SOCKET_STATUS_CLOSED) {
        uv_mutex_unlock(&sock->mutex_handle);
        return COBRA_SOCKET_ERR_NOT_CLOSED;
    }

    sock->connection_status = COBRA_SOCKET_STATUS_RESOLVING;

    uv_getaddrinfo_t *resolve_request = malloc(sizeof(uv_getaddrinfo_t));
    resolve_request->data = sock;

    uv_getaddrinfo(&sock->loop,
                   resolve_request,
                   cobra__socket_resolve_callback,
                   host,
                   port,
                   NULL);

    uv_mutex_unlock(&sock->mutex_handle);
    uv_run(&sock->loop, UV_RUN_DEFAULT);

    return COBRA_SOCKET_OK;
}

void cobra__socket_resolve_callback(uv_getaddrinfo_t *resolve_request,
                                    int error,
                                    struct addrinfo *addrinfo) {
    cobra_socket_t *sock = resolve_request->data;

    if (error) {
        // TODO: Close socket connection with resolving error
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);
    sock->connection_status = COBRA_SOCKET_STATUS_CONNECTING;

    uv_connect_t *connect_request = malloc(sizeof(uv_connect_t));
    connect_request->data = sock;

    uv_tcp_connect(connect_request,
                   &sock->tcp_handle,
                   addrinfo->ai_addr,
                   cobra__socket_connect_callback);

    uv_mutex_unlock(&sock->mutex_handle);

    // Specified in libuv docs
    uv_freeaddrinfo(addrinfo);
    free(resolve_request);
}

void cobra__socket_connect_callback(uv_connect_t *connect_request,
                                    int error) {
    cobra_socket_t *sock = connect_request->data;

    if (error) {
        // TODO: Close socket connection with connecting error
        return;
    }

    uv_mutex_lock(&sock->mutex_handle);
    sock->connection_status = COBRA_SOCKET_STATUS_CONNECTED;

    if (sock->connect_callback) {
        uv_mutex_unlock(&sock->mutex_handle);
        sock->connect_callback(sock);
        uv_mutex_lock(&sock->mutex_handle);
    }

    uv_read_start((uv_stream_t *) &sock->tcp_handle,
                  cobra__socket_alloc_callback,
                  cobra__socket_read_callback);

    uv_mutex_unlock(&sock->mutex_handle);
    free(connect_request);
}